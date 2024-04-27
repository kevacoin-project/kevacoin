// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <arith_uint256.h>
#include <chainparams.h>
#include <consensus/merkle.h>

#include <tinyformat.h>
#include <util.h>
#include <utilstrencodings.h>

#include <assert.h>
#include <memory>

#include <chainparamsseeds.h>

/**
 * For Test net and Reg Test.
 */
static CBlock CreateGenesisBlockTest(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = 0; // Used as height.
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);

    genesis.cnHeader.major_version  = 10; // Cryptonight variant 4
    genesis.cnHeader.prev_id        = genesis.GetOriginalBlockHash();
    genesis.cnHeader.nonce          = nNonce;
    return genesis;
}

static CBlock CreateGenesisBlockTest(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "The Economist 27/Sept/2019 Repo-market ructions were a reminder of the financial crisis";
    const CScript genesisOutputScript = CScript() << ParseHex("a914676a24ba4bfadd458e5245b26fa57f9a62ca185087");
    return CreateGenesisBlockTest(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

/**
 * For Main net.
 */
static CBlock CreateGenesisBlockMain(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = 0; // Used as height.
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);

    genesis.cnHeader.major_version  = 10; // Cryptonight variant 4
    genesis.cnHeader.minor_version  = 0;
    genesis.cnHeader.prev_id        = genesis.GetOriginalBlockHash();
    genesis.cnHeader.merkle_root    = uint256S("0x09bafe2103d3588f80ef5a876f3b24fc1fc277d7105798e163600652dc02de6f");
    genesis.cnHeader.nonce          = nNonce;
    genesis.cnHeader.timestamp      = nTime;
    genesis.cnHeader.nTxes          = 1;
    return genesis;
}

static CBlock CreateGenesisBlockMain(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "Thank You Satoshi 612997 2020-01-15 11:40:41 7f31f44d";
    const CScript genesisOutputScript = CScript() << ParseHex("a914676a24ba4bfadd458e5245b26fa57f9a62ca185087");
    return CreateGenesisBlockMain(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

void CChainParams::UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
    consensus.vDeployments[d].nStartTime = nStartTime;
    consensus.vDeployments[d].nTimeout = nTimeout;
}

/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */

class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";
        consensus.nSubsidyHalvingInterval = 1050000; // 210000 * 5
        consensus.BIP16Height = 1;
        consensus.BIP34Height = 1;
        consensus.BIP65Height = 1;
        consensus.BIP66Height = 1;
        consensus.RandomXHeight = 46130;
        consensus.powLimit = uint256S("000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 2.0 * 60; // Two minutes
        consensus.nPowTargetSpacing = 2.0 * 60; // Two minutes
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1620; // 75% of 2160
        consensus.nMinerConfirmationWindow = 2160; // 3 days
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = -1; // Consensus::BIP9Deployment::ALWAYS_ACTIVE
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1517356801; // January 31st, 2018

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = -1; // Consensus::BIP9Deployment::ALWAYS_ACTIVE
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1517356801; // January 31st, 2018

        // Deployment of NsFix
        consensus.vDeployments[Consensus::DEPLOYMENT_NSFIX].bit = 3;
        consensus.vDeployments[Consensus::DEPLOYMENT_NSFIX].nStartTime = 1594771200; // 07/15/2020 @ 12:00am
        consensus.vDeployments[Consensus::DEPLOYMENT_NSFIX].nTimeout = 1598745600; // 08/30/2020 @ 12:00am

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork  = uint256S("0x00000000000000000000000000000000000000000000000000015aa720175d79");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00");

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0x6a;
        pchMessageStart[1] = 0xc6;
        pchMessageStart[2] = 0x07;
        pchMessageStart[3] = 0x9a;
        nDefaultPort = 9338;
        nPruneAfterHeight = 100000;

        const uint32_t genesisBlockReward = 0.00001 * COIN; // A small reward for the core developers :-)
        genesis = CreateGenesisBlockMain(1579143600, 585290, 0x1e0fffff, 1, genesisBlockReward);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x70bd30ae775c691fc8a2b7d27f37279a4f505f877e3234105f22e963a618597c"));
        assert(genesis.hashMerkleRoot == uint256S("0xe6104a982da24d09ccf867aba92abbd31b2ede9da636941367709c5ef24d3330"));

        // Note that of those with the service bits flag, most only support a subset of possible options
        vSeeds.emplace_back("dnsseed.kevacoin.org");
        vSeeds.emplace_back("dnsseed.keva.one");

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,45); // K
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,5);
        base58Prefixes[SCRIPT_ADDRESS2] = std::vector<unsigned char>(1,70); // V
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,139); // M
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};
        base58Prefixes[KEVA_NAMESPACE] = std::vector<unsigned char>(1,53); // N

        bech32_hrp = "kva";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;

        checkpointData = {
            {
                { 11111, uint256S("0xbdb54e64bfb0e1026e867b05fc873eb96b60ec71533b45522be2f39ccd93b425")},
                { 77777, uint256S("0xa512d8650ca2292b85145a0559bb5d09160b62c6388a74087114245f0beb4c9e")},
                { 111111, uint256S("0x90327d8bfcc9739cbbb71fe02cd4cd3aeef8c411fb24d934298bcb5399e6abb1")},
                { 222222, uint256S("0x93a4ae3a19651caeff9d8cb589e294e18bce40e0c91c1f616c4b8dd92d6b24ca")},
                { 333333, uint256S("0xe61c9548ac0c5bfe43cc04834e192a4bbb5460fe09120bf566b84fe975a625ac")},
                { 420000, uint256S("0xabee716c8c46398a6500a413698ee40f9a3ef702a7e42fedfcb57ff2bf2cff9c")},
                { 470000, uint256S("0xf9e8339cde8763538fbdf58dd0ff8d3ac9aebab03aa12d6dfb95340f5f07aa79")},
                { 500000, uint256S("0x5fb5c7583b321f915d71f50dd6a58bb808ca94a66a523706d97a4e6caffa4ce2")},
            }
        };

        chainTxData = ChainTxData{
            // Data from rpc: getchaintxstats 4096
            /* nTime    */ 1647739398,
            /* nTxCount */ 922450,
            /* dTxRate  */ 0.01457148131301816
        };
    }

    int DefaultCheckKevaDB() const
    {
        return -1;
    }
};

/**
 * Testnet (v7)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        consensus.nSubsidyHalvingInterval = 840000;
        consensus.BIP16Height = 0; // always enforce P2SH BIP16 on regtest
        consensus.BIP34Height = 1;
        consensus.BIP34Hash = uint256S("0xa8dbaa66a9266348f6527fe528efea73227d51938befb81d5a1521cebd319c4a"); // Genesis
        consensus.BIP65Height = 1;
        consensus.BIP66Height = 1;
        consensus.RandomXHeight = 3;
        consensus.powLimit = uint256S("000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 2.0 * 60; // Two minutes
        consensus.nPowTargetSpacing = 2.0 * 60; // Two minutes
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
        consensus.nMinerConfirmationWindow = 144; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = -1; // Consensus::BIP9Deployment::ALWAYS_ACTIVE
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1517356801; // January 31st, 2018

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = -1; // Consensus::BIP9Deployment::ALWAYS_ACTIVE
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1517356801; // January 31st, 2018

        // Deployment of NsFix
        consensus.vDeployments[Consensus::DEPLOYMENT_NSFIX].bit = 3;
        consensus.vDeployments[Consensus::DEPLOYMENT_NSFIX].nStartTime = 1594771200; // 07/15/2020 @ 12:00am
        consensus.vDeployments[Consensus::DEPLOYMENT_NSFIX].nTimeout = 1598745600; // 08/30/2020 @ 12:00am

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork  = uint256S("0x0000000000000000000000000000000000000000000000000000000000001000");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00");

        pchMessageStart[0] = 0xfe;
        pchMessageStart[1] = 0xec;
        pchMessageStart[2] = 0x65;
        pchMessageStart[3] = 0xe4;
        nDefaultPort = 19335;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlockTest(1582997614, 3536, 0x1f0ffff0, 1, 500 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("21ebe179e22753b30e605f0381f6a313cdd16d09b2310db8d5bc87ada743269f"));
        assert(genesis.hashMerkleRoot == uint256S("d85a90623fbff6a5ea4b80df1dbc81b32de7f1011f484e186cfb7cf2d4292c95"));

        vFixedSeeds.clear();
        vSeeds.clear();
        // nodes with support for servicebits filtering should be at the top
        vSeeds.emplace_back("testnet-seed.kevacoin.org");

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,55); // P
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,5);
        base58Prefixes[SCRIPT_ADDRESS2] = std::vector<unsigned char>(1,65); // T
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,58); // 9
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};
        base58Prefixes[KEVA_NAMESPACE] = std::vector<unsigned char>(1,53); // N

        bech32_hrp = "tkva";
        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;

        checkpointData = (CCheckpointData) {
            {
                {0, uint256S("bf7757238413571d9686ffc4899469affeb7675d4fb23693b88646d482fe12f6")},
            }
        };

        chainTxData = ChainTxData{
        };

    }

    int DefaultCheckKevaDB() const
    {
        return -1;
    }
};

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";
        consensus.nSubsidyHalvingInterval = 150;
        consensus.BIP16Height = 0; // always enforce P2SH BIP16 on regtest
        consensus.BIP34Height = 100000000; // BIP34 has not activated on regtest (far in the future so block v1 are not rejected in tests)
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = 1351; // BIP65 activated on regtest (Used in rpc activation tests)
        consensus.BIP66Height = 1251; // BIP66 activated on regtest (Used in rpc activation tests)
        consensus.RandomXHeight = 2000000; // RandomX acticated on regtest.
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 2.0 * 60; // Two minutes
        consensus.nPowTargetSpacing = 2.0 * 60; // Two minutes
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
        consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        //consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        //consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;

        // Deployment of NsFix
        consensus.vDeployments[Consensus::DEPLOYMENT_NSFIX].bit = 3;
        consensus.vDeployments[Consensus::DEPLOYMENT_NSFIX].nStartTime = 1517356801; // January 31st, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_NSFIX].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00");

        pchMessageStart[0] = 0xfa;
        pchMessageStart[1] = 0xbf;
        pchMessageStart[2] = 0xb5;
        pchMessageStart[3] = 0xda;
        nDefaultPort = 19444;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlockTest(1582959692, 0, 0x207fffff, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x913ef11843ce0d4905bfdbd73c8e57bc9b35619dfada60474e64040b351b9941"));
        assert(genesis.hashMerkleRoot == uint256S("0xb4db37fbc5c9ce5396b7d14dba94460cbcb4cf16e932b3cb6aece2244babf5b9"));

        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.

        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;

        checkpointData = {
            {
                {0, uint256S("5b2e996d458adbf5c81b381f90ca167732bc9f4e9c1c4ec8485fa74efe793ed8")},
            }
        };

        chainTxData = ChainTxData{
            0,
            0,
            0
        };

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SCRIPT_ADDRESS2] = std::vector<unsigned char>(1,58);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};
        base58Prefixes[KEVA_NAMESPACE] = std::vector<unsigned char>(1,53); // N

        bech32_hrp = "rkva";
    }

    int DefaultCheckKevaDB() const
    {
        return 0;
    }
};

static std::unique_ptr<CChainParams> globalChainParams;

const CChainParams &Params() {
    assert(globalChainParams);
    return *globalChainParams;
}

std::unique_ptr<CChainParams> CreateChainParams(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
        return std::unique_ptr<CChainParams>(new CMainParams());
    else if (chain == CBaseChainParams::TESTNET)
        return std::unique_ptr<CChainParams>(new CTestNetParams());
    else if (chain == CBaseChainParams::REGTEST)
        return std::unique_ptr<CChainParams>(new CRegTestParams());
    throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    globalChainParams = CreateChainParams(network);
}

void UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
    globalChainParams->UpdateVersionBitsParameters(d, nStartTime, nTimeout);
}
