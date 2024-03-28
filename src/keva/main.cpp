// Copyright (c) 2014-2017 Daniel Kraft
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

// Copyright (c) 2018 the Kevacoin Core Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <keva/main.h>
#include "base58.h"

#include <chainparams.h>
#include <coins.h>
#include <consensus/validation.h>
#include <hash.h>
#include <dbwrapper.h>
#include <script/interpreter.h>
#include <script/keva.h>
#include <txmempool.h>
#include <undo.h>
#include <util.h>
#include <utilstrencodings.h>
#include <validation.h>
#include <validationinterface.h>


/* ************************************************************************** */
/* CKevaTxUndo.  */

void
CKevaTxUndo::fromOldState(const valtype& nameSpace, const valtype& key, const CCoinsView& view)
{
  this->nameSpace = nameSpace;
  this->key = key;
  isNew = !view.GetName(nameSpace, key, oldData);
}

void
CKevaTxUndo::apply(CCoinsViewCache& view) const
{
  if (isNew) {
    CKevaData oldData;
    if (view.GetName(nameSpace, key, oldData)) {
      view.DeleteKey(nameSpace, key);
    }
  } else {
    view.SetKeyValue(nameSpace, key, oldData, true);
  }
}

/* ************************************************************************** */
/* CKevaMemPool.  */

void
CKevaMemPool::addUnchecked(const uint256& hash, const CKevaScript& kevaOp)
{
  AssertLockHeld (pool.cs);
  if (kevaOp.isNamespaceRegistration()) {
    const valtype& nameSpace = kevaOp.getOpNamespace();
    listUnconfirmedNamespaces.push_back(std::make_tuple(hash, nameSpace, kevaOp.getOpNamespaceDisplayName()));
  }

  if (kevaOp.getKevaOp() == OP_KEVA_PUT) {
    const valtype& nameSpace = kevaOp.getOpNamespace();
    listUnconfirmedKeyValues.push_back(std::make_tuple(hash, nameSpace, kevaOp.getOpKey(), kevaOp.getOpValue()));
  }

  if (kevaOp.getKevaOp() == OP_KEVA_DELETE) {
    const valtype& nameSpace = kevaOp.getOpNamespace();
    const valtype& empty = ValtypeFromString("");
    listUnconfirmedKeyValues.push_back(std::make_tuple(hash, nameSpace, kevaOp.getOpKey(), empty));
  }
}

bool
CKevaMemPool::getUnconfirmedKeyValue(const valtype& nameSpace, const valtype& key, valtype& value) const {
  bool found = false;
  for (auto entry : listUnconfirmedKeyValues) {
    auto ns = std::get<1>(entry);
    auto k = std::get<2>(entry);
    if (ns == nameSpace && key == k) {
      value = std::get<3>(entry);
      found = true;
    }
  }
  return found;
}

void
CKevaMemPool::getUnconfirmedKeyValueList(std::vector<std::tuple<valtype, valtype, valtype, uint256>>& keyValueList, const valtype& nameSpace) {
  bool matchNamespace = nameSpace.size() > 0;
  for (auto entry : listUnconfirmedKeyValues) {
    auto txid = std::get<0>(entry);
    auto n = std::get<1>(entry);
    auto k = std::get<2>(entry);
    auto v = std::get<3>(entry);
    if (!matchNamespace) {
      keyValueList.push_back(std::make_tuple(n, k, v, txid));
    } else if (n == nameSpace) {
      keyValueList.push_back(std::make_tuple(n, k, v, txid));
    }
  }
}

void
CKevaMemPool::getUnconfirmedNamespaceList(std::vector<std::tuple<valtype, valtype, uint256>>& nameSpaces) const {
  for (auto entry : listUnconfirmedNamespaces) {
    auto txid = std::get<0>(entry);
    auto ns = std::get<1>(entry);
    auto displayName = std::get<2>(entry);
    nameSpaces.push_back(std::make_tuple(ns, displayName, txid));
  }
}

void CKevaMemPool::remove(const CTxMemPoolEntry& entry)
{
  AssertLockHeld (pool.cs);
  if (entry.isNamespaceRegistration()) {
    auto hash = entry.GetTx().GetHash();
    for (auto iter = listUnconfirmedNamespaces.begin(); iter != listUnconfirmedNamespaces.end(); ++iter) {
      if (std::get<0>(*iter) == hash) {
        listUnconfirmedNamespaces.erase(iter);
        break;
      }
    }
  }

  if (entry.isKeyUpdate() || entry.isKeyDelete()) {
    auto hash = entry.GetTx().GetHash();
    for (auto iter = listUnconfirmedKeyValues.begin(); iter != listUnconfirmedKeyValues.end(); ++iter) {
      if (std::get<0>(*iter) == hash) {
        listUnconfirmedKeyValues.erase(iter);
        break;
      }
    }
  }
}

void
CKevaMemPool::removeConflicts(const CTransaction& tx)
{
}

bool CKevaMemPool::validateNamespace(const CTransaction& tx, const valtype& nameSpace) const
{
  if (tx.vin.size() == 0) {
    return false;
  }
  valtype kevaNamespace;
  bool nsFixEnabled = IsNsFixEnabled(chainActive.Tip(), Params().GetConsensus());
  CKevaScript::generateNamespace(tx.vin[0].prevout.hash, tx.vin[0].prevout.n, kevaNamespace, Params(), nsFixEnabled);
  return kevaNamespace == nameSpace;
}

bool
CKevaMemPool::checkTx(const CTransaction& tx) const
{
  AssertLockHeld(pool.cs);

  if (!tx.IsKevacoin()) {
    return true;
  }

  for (const auto& txout : tx.vout) {
    const CKevaScript nameOp(txout.scriptPubKey);
    if (!nameOp.isKevaOp())
      continue;
    switch (nameOp.getKevaOp()) {
      case OP_KEVA_NAMESPACE:
      {
        const valtype& nameSpace = nameOp.getOpNamespace();
        std::map<valtype, uint256>::const_iterator mi;
        if (!validateNamespace(tx, nameSpace)) {
          return false;
        }
        break;
      }

      case OP_KEVA_PUT:
      {
        break;
      }

      case OP_KEVA_DELETE:
      {
        break;
      }

      default:
        assert (false);
    }
  }

  return true;
}

/* ************************************************************************** */
/* CNameConflictTracker.  */

namespace
{

void
ConflictTrackerNotifyEntryRemoved (CNameConflictTracker* tracker,
                                   CTransactionRef txRemoved,
                                   MemPoolRemovalReason reason)
{
  if (reason == MemPoolRemovalReason::KEVA_CONFLICT)
    tracker->AddConflictedEntry (txRemoved);
}

} // anonymous namespace

CNameConflictTracker::CNameConflictTracker (CTxMemPool &p)
  : txNameConflicts(std::make_shared<std::vector<CTransactionRef>>()), pool(p)
{
  pool.NotifyEntryRemoved.connect (
    boost::bind (&ConflictTrackerNotifyEntryRemoved, this, boost::placeholders::_1, boost::placeholders::_2));
}

CNameConflictTracker::~CNameConflictTracker ()
{
  pool.NotifyEntryRemoved.disconnect (
    boost::bind (&ConflictTrackerNotifyEntryRemoved, this, boost::placeholders::_1, boost::placeholders::_2));
}

void
CNameConflictTracker::AddConflictedEntry (CTransactionRef txRemoved)
{
  txNameConflicts->emplace_back (std::move (txRemoved));
}

/* ************************************************************************** */

CKevaNotifier::CKevaNotifier(CMainSignals* signals) {
  this->signals = signals;
}

void CKevaNotifier::KevaNamespaceCreated(const CTransaction& tx, const CBlockIndex &pindex, const std::string& nameSpace) {
  if (signals) {
    CTransactionRef ptx = MakeTransactionRef(tx);
    signals->KevaNamespaceCreated(ptx, pindex, nameSpace);
  }
}

void CKevaNotifier::KevaUpdated(const CTransaction& tx, const CBlockIndex &pindex, const std::string& nameSpace, const std::string& key, const std::string& value) {
  if (signals) {
    CTransactionRef ptx = MakeTransactionRef(tx);
    signals->KevaUpdated(ptx, pindex, nameSpace, key, value);
  }
}

void CKevaNotifier::KevaDeleted(const CTransaction& tx, const CBlockIndex &pindex, const std::string& nameSpace, const std::string& key) {
  if (signals) {
    CTransactionRef ptx = MakeTransactionRef(tx);
    signals->KevaDeleted(ptx, pindex, nameSpace, key);
  }
}

/* ************************************************************************** */

bool
CheckKevaTransaction (const CTransaction& tx, unsigned nHeight,
                      const CCoinsView& view,
                      CValidationState& state, unsigned flags)
{
  const std::string strTxid = tx.GetHash ().GetHex ();
  const char* txid = strTxid.c_str ();

  /*
    As a first step, try to locate inputs and outputs of the transaction
    that are keva scripts.  At most one input and output should be
    a keva operation.
  */

  int nameIn = -1;
  CKevaScript nameOpIn;
  Coin coinIn;
  for (unsigned i = 0; i < tx.vin.size(); ++i) {
    const COutPoint& prevout = tx.vin[i].prevout;
    Coin coin;
    if (!view.GetCoin(prevout, coin)) {
      return error("%s: failed to fetch input coin for %s", __func__, txid);
    }

    const CKevaScript op(coin.out.scriptPubKey);
    if (op.isKevaOp()) {
      if (nameIn != -1) {
        return state.Invalid(error("%s: multiple name inputs into transaction %s", __func__, txid));
      }
      nameIn = i;
      nameOpIn = op;
      coinIn = coin;
    }
  }

  int nameOut = -1;
  CKevaScript nameOpOut;
  for (unsigned i = 0; i < tx.vout.size(); ++i) {
    const CKevaScript op(tx.vout[i].scriptPubKey);
    if (op.isKevaOp()) {
      if (nameOut != -1) {
        return state.Invalid(error("%s: multiple name outputs from transaction %s", __func__, txid));
      }
      nameOut = i;
      nameOpOut = op;
    }
  }

  /*
    Check that no keva inputs/outputs are present for a non-Kevacoin tx.
    If that's the case, all is fine.  For a kevacoin tx instead, there
    should be at least an output.
  */

  if (!tx.IsKevacoin()) {
    if (nameIn != -1) {
      return state.Invalid(error("%s: non-Kevacoin tx %s has keva inputs", __func__, txid));
    }
    if (nameOut != -1) {
      return state.Invalid (error ("%s: non-Kevacoin tx %s at height %u has keva outputs",
                                     __func__, txid, nHeight));
    }
    return true;
  }

  assert(tx.IsKevacoin());
  if (nameOut == -1) {
    return state.Invalid (error ("%s: Kevacoin tx %s has no keva outputs", __func__, txid));
  }

  /* Reject "greedy names".  */
  if (tx.vout[nameOut].nValue < KEVA_LOCKED_AMOUNT) {
    return state.Invalid (error ("%s: greedy name", __func__));
  }

  if (nameOpOut.isNamespaceRegistration()) {
    if (nameOpOut.getOpNamespaceDisplayName().size () > MAX_VALUE_LENGTH) {
      return state.Invalid (error ("CheckKevaTransaction: display name value too long"));
    }

    LOCK(cs_main);
    bool nsFixEnabled = IsNsFixEnabled(chainActive.Tip(), Params().GetConsensus());
    if (!nsFixEnabled) {
      // This is a historic bug.
      return true;
    }
    // Make sure the namespace Id is correctly derived from vin[0].
    valtype expectedNamespace;
    CKevaScript::generateNamespace(tx.vin[0].prevout.hash, tx.vin[0].prevout.n, expectedNamespace, Params(), true);
    return expectedNamespace == nameOpOut.getOpNamespace();
  }

  assert(nameOpOut.isAnyUpdate());

  if (nameIn == -1) {
    return state.Invalid(error("CheckKevaTransaction: update without previous keva input"));
  }

  const valtype& key = nameOpOut.getOpKey();
  if (key.size() > MAX_KEY_LENGTH) {
    return state.Invalid (error ("CheckKevaTransaction: key too long"));
  }

  const valtype& nameSpace = nameOpOut.getOpNamespace();
  if (nameSpace != nameOpIn.getOpNamespace()) {
    return state.Invalid(error("%s: KEVA_PUT namespace mismatch to prev tx found in %s", __func__, txid));
  }

  if (nameOpOut.getKevaOp() == OP_KEVA_PUT) {
    if (nameOpOut.getOpValue().size () > MAX_VALUE_LENGTH) {
      return state.Invalid (error ("CheckKevaTransaction: value too long"));
    }

    if (!nameOpIn.isAnyUpdate() && !nameOpIn.isNamespaceRegistration()) {
      return state.Invalid(error("CheckKevaTransaction: KEVA_PUT with prev input that is no update"));
    }
  }

  if (nameOpOut.getKevaOp() == OP_KEVA_DELETE) {
    if (!nameOpIn.isAnyUpdate() && !nameOpIn.isNamespaceRegistration()) {
      return state.Invalid(error("CheckKevaTransaction: KEVA_DELETE with prev input that is no update"));
    }
  }
  return true;
}

void ApplyKevaTransaction(const CTransaction& tx, const CBlockIndex& pindex,
                        CCoinsViewCache& view, CBlockUndo& undo, CKevaNotifier& notifier)
{
  unsigned int nHeight = pindex.nHeight;
  assert (nHeight != MEMPOOL_HEIGHT);
  if (!tx.IsKevacoin())
    return;

  /* Changes are encoded in the outputs.  We don't have to do any checks,
     so simply apply all these.  */

  for (unsigned i = 0; i < tx.vout.size(); ++i) {
    const CKevaScript op(tx.vout[i].scriptPubKey);
    if (!op.isKevaOp()) {
      continue;
    }
    if (op.isNamespaceRegistration()) {
      const valtype& nameSpace = op.getOpNamespace();
      const valtype& displayName = op.getOpNamespaceDisplayName();
      LogPrint (BCLog::KEVA, "Register name at height %d: %s, display name: %s\n",
                nHeight, ValtypeToString(nameSpace).c_str(),
                ValtypeToString(displayName).c_str());

      const valtype& key = ValtypeFromString(CKevaScript::KEVA_DISPLAY_NAME_KEY);
      CKevaTxUndo opUndo;
      opUndo.fromOldState(nameSpace, key, view);
      undo.vkevaundo.push_back(opUndo);

      CKevaData data;
      data.fromScript(nHeight, COutPoint(tx.GetHash(), i), op);
      view.SetKeyValue(nameSpace, key, data, false);
      notifier.KevaNamespaceCreated(tx, pindex, EncodeBase58Check(nameSpace));
    } else if (op.isAnyUpdate()) {
      const valtype& nameSpace = op.getOpNamespace();
      const valtype& key = op.getOpKey();
      LogPrint (BCLog::KEVA, "Updating key at height %d: %s %s\n",
                nHeight, ValtypeToString(nameSpace).c_str(), ValtypeToString(key).c_str());

      CKevaTxUndo opUndo;
      opUndo.fromOldState(nameSpace, key, view);
      undo.vkevaundo.push_back(opUndo);

      CKevaData data;
      if (op.isDelete()) {
        CKevaData oldData;
        if (view.GetName(nameSpace, key, oldData)) {
          view.DeleteKey(nameSpace, key);
          notifier.KevaDeleted(tx, pindex, EncodeBase58Check(nameSpace), ValtypeToString(key));
        }
      } else {
        data.fromScript(nHeight, COutPoint(tx.GetHash(), i), op);
        view.SetKeyValue(nameSpace, key, data, false);
        notifier.KevaUpdated(tx, pindex, EncodeBase58Check(nameSpace), ValtypeToString(key), ValtypeToString(data.getValue()));
      }
    }
  }
}

void
CheckNameDB (bool disconnect)
{
  const int option
    = gArgs.GetArg ("-checkkevadb", Params().DefaultCheckKevaDB ());

  if (option == -1)
    return;

  assert (option >= 0);
  if (option != 0)
    {
      if (disconnect || chainActive.Height () % option != 0)
        return;
    }

  pcoinsTip->Flush ();
  const bool ok = pcoinsTip->ValidateKevaDB();

  /* The DB is inconsistent (mismatch between UTXO set and names DB) between
     (roughly) blocks 139,000 and 180,000.  This is caused by libcoin's
     "name stealing" bug.  For instance, d/postmortem is removed from
     the UTXO set shortly after registration (when it is used to steal
     names), but it remains in the name DB until it expires.  */
  if (!ok)
    {
      const unsigned nHeight = chainActive.Height ();
      LogPrintf ("ERROR: %s : name database is inconsistent\n", __func__);
      if (nHeight >= 139000 && nHeight <= 180000)
        LogPrintf ("This is expected due to 'name stealing'.\n");
      else
        assert (false);
    }
}
