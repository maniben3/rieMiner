// (c) 2017-2018 Pttn (https://github.com/Pttn/rieMiner)

#ifndef HEADER_Client_hpp
#define HEADER_Client_hpp

#include <vector>
#include <memory>
#include <mutex>
#include <jansson.h>
#include <curl/curl.h>
#include "tools.hpp"

class WorkManager;

struct BlockHeader { // Total 1024 bits/128 bytes (256 hex chars)
	uint32_t version;
	uint8_t  previousblockhash[32]; // 256 bits
	uint8_t  merkleRoot[32];        // 256 bits
	uint32_t bits;
	uint64_t curtime;               // Riecoin has 64 bits timestamps
	uint8_t  nOffset[32];           // 256 bits
	uint8_t  remaining[16];         // 128 bits
	
	BlockHeader() {
		version = 0;
		bits = 0;
		curtime = 0;
		for (uint8_t i(0) ; i < 8 ; i++) {
			previousblockhash[i] = 0;
			merkleRoot[i] = 0;
			nOffset[i] = 0;
			if (i < 4) remaining[i] = 0;
		}
	}
	
	mpz_class decodeSolution() const {
		const std::string bhStr(binToHexStr(this, 112));
		const uint32_t diff(getCompact(invEnd32(strtol(bhStr.substr(136, 8).c_str(), NULL, 16))));
		std::vector<uint8_t> SV8(32), XV8, tmp(sha256sha256(hexStrToV8(bhStr.substr(0, 160)).data(), 80));
		for (uint64_t i(0) ; i < 256 ; i++) SV8[i/8] |= (((tmp[i/8] >> (i % 8)) & 1) << (7 - (i % 8)));
		mpz_class S(v8ToHexStr(SV8).c_str(), 16), target(1);
		mpz_mul_2exp(S.get_mpz_t(), S.get_mpz_t(), diff - 265);
		mpz_mul_2exp(target.get_mpz_t(), target.get_mpz_t(), diff - 1);
		target += S;
		XV8 = reverse(hexStrToV8(bhStr.substr(160, 64)));
		mpz_class X(v8ToHexStr(XV8).c_str(), 16);
		return target + X;
	}
};

// Stores all the information needed for the miner and submissions
struct WorkData {
	BlockHeader bh;
	uint32_t height, targetCompact;
	uint16_t primes;
	
	// For GetBlockTemplate
	std::string transactions; // Store the concatenation in hex format
	uint16_t txCount;
	
	// For Stratum
	std::vector<uint8_t> extraNonce1, extraNonce2;
	std::string jobId;
	
	WorkData() {
		bh = BlockHeader();
		height = 0;
		targetCompact = 0;
		primes = 0;
		
		transactions = std::string();
		txCount = 0;
		
		extraNonce1 = std::vector<uint8_t>();
		extraNonce2 = std::vector<uint8_t>();
		jobId = std::string();
	}
};

// Communicates with the server to get, parse, and submit mining work
// Absctract class with protocol independent member variables and functions
class Client {
	protected:
	bool _inited, _connected;
	CURL *_curl;
	std::mutex _submitMutex;
	std::vector<WorkData> _pendingSubmissions;
	
	std::shared_ptr<WorkManager> _manager;
	
	public:
	Client() {_inited = false;}
	Client(const std::shared_ptr<WorkManager>&);
	virtual bool connect(); // Returns false on error or if already connected
	virtual bool getWork() = 0; // Get work (block data,...) from the sever, depending on the chosen protocol
	virtual void sendWork(const WorkData&) const = 0;  // Send work (share or block) to the sever, depending on the chosen protocol
	void addSubmission(const WorkData& work) {
		_submitMutex.lock();
		_pendingSubmissions.push_back(work);
		_submitMutex.unlock();
	}
	virtual bool process(); // Processes submissions and updates work
	bool connected() const {return _connected;}
	// The WorkManager will get the work ready to send to the miner using this
	// In particular, will do the needed endianness changes or randomizations
	virtual WorkData workData() const = 0; // If the returned work data has height 0, it is invalid
};

class RPCClient : public Client {
	public:
	using Client::Client;
	std::string getUserPass() const;
	std::string getHostPort() const;
	json_t* sendRPCCall(const std::string&) const; // Send a RPC call to the server
};

class BMClient : public Client {
	BlockHeader _bh;
	uint32_t _height;
	
	public:
	using Client::Client;
	bool connect();
	bool getWork();
	void sendWork(const WorkData&) const;
	WorkData workData() const;
};

#endif
