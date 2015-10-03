/*
*
* Copyright 2015 Telefonica Investigacion y Desarrollo, S.A.U
*
* This file is part of Orion Context Broker.
*
* Orion Context Broker is free software: you can redistribute it and/or
* modify it under the terms of the GNU Affero General Public License as
* published by the Free Software Foundation, either version 3 of the
* License, or (at your option) any later version.
*
* Orion Context Broker is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero
* General Public License for more details.
*
* You should have received a copy of the GNU Affero General Public License
* along with Orion Context Broker. If not, see http://www.gnu.org/licenses/.
*
* For those usages not covered by this license please contact with
* iot_support at tid dot es
*
* Author: Orion dev team
*/

#include "common/sem.h"

#include "logMsg/logMsg.h"
#include "logMsg/traceLevels.h"

#include "mongoBackend/mongoListSubscriptions.h"
#include "mongoBackend/MongoGlobal.h"
#include "mongoBackend/collectionOperations.h"

#include "mongo/client/dbclient.h"

using namespace ngsiv2;

/* ****************************************************************************
*
* setSubscriptionId -
*/
static void setSubscriptionId(Subscription* s, const BSONObj& r)
{
  s->id = r.getField("_id").OID().toString();
}


/* ****************************************************************************
*
* setSubject -
*/
static void setSubject(Subscription* s, const BSONObj& r)
{
  // Entities
  std::vector<BSONElement> ents = r.getField(CSUB_ENTITIES).Array();
  for (unsigned int ix = 0; ix < ents.size(); ++ix)
  {
    BSONObj ent           = ents[ix].embeddedObject();
    std::string id        = ent.getStringField(CSUB_ENTITY_ID);
    std::string type      = ent.getStringField(CSUB_ENTITY_TYPE);
    std::string isPattern = ent.getStringField(CSUB_ENTITY_ISPATTERN);

    EntID en;
    if (isFalse(isPattern))
    {
      en.id = id;
    }
    else
    {
      en.idPattern = id;
    }
    en.type = type;

    s->subject.entities.push_back(en);
  }

  // Condition
  std::vector<BSONElement> conds = r.getField(CSUB_CONDITIONS).Array();
  for (unsigned int ix = 0; ix < conds.size(); ++ix)
  {
    BSONObj cond = conds[ix].embeddedObject();
    // The ONCHANGE check is needed, as a subscription could mix different conditions types in DB
    if (std::string(cond.getStringField(CSUB_CONDITIONS_TYPE)) == "ONCHANGE")
    {
      std::vector<BSONElement> condValues = cond.getField(CSUB_CONDITIONS_VALUE).Array();
      for (unsigned int jx = 0; jx < condValues.size(); ++jx)
      {
        std::string attr = condValues[jx].String();
        s->subject.condition.attributes.push_back(attr);
      }
    }
  }

  // Note that current DB model is based on NGSIv1 and doesn't consider expressions. Thus
  // subject.condition.expression cannot be filled. The implemetion will be enhanced once
  // the DB model gets defined
  // TBD

}


/* ****************************************************************************
*
* setNotification -
*/
static void setNotification(Subscription* s, const BSONObj& r)
{
  // Attributes
  std::vector<BSONElement> attrs = r.getField(CSUB_ATTRS).Array();
  for (unsigned int ix = 0; ix < attrs.size(); ++ix)
  {
    std::string attr = attrs[ix].String();
    s->notification.attributes.push_back(attr);
  }

  // Callback
  s->notification.callback = r.getStringField(CSUB_REFERENCE);

  // Throttling
  long long throttling = r.hasField(CSUB_THROTTLING)? r.getField(CSUB_THROTTLING).numberLong() : -1;
  if (throttling > -1)
  {
    // FIXME P10: int -> str conversion needed
    s->notification.throttling.set("FIXME");
  }
}


/* ****************************************************************************
*
* setDuration -
*/
static void setDuration(Subscription* s, const BSONObj& r)
{
  //long long expiration = r.hasField(CSUB_EXPIRATION)? r.getField(CSUB_EXPIRATION).numberLong() : 0;
  // FIXME P10: int -> str conversion needed
  s->duration.set("FIXME");
}

/* ****************************************************************************
*
* mongoListSubscriptions -
*/
OrionError mongoListSubscriptions
(
  std::vector<Subscription>&           subs,
  std::map<std::string, std::string>&  uriParam,
  const std::string&                   tenant
)
{

  // FIXME P10: Pagination not yet implemented

  bool           reqSemTaken = false;

  reqSemTake(__FUNCTION__, "Mongo List Subscriptions", SemReadOp, &reqSemTaken);

  LM_T(LmtMongo, ("Mongo List Subscriptions"));

  /* ONTIMEINTERVAL subscription are not part of NGSIv2, so they are excluded.
   * Note that expiration is not taken into account (in the future, a q= query
   * could be added to the operation in order to filter results) */
  std::auto_ptr<DBClientCursor>  cursor;
  std::string                    err;
  std::string                    conds = std::string(CSUB_CONDITIONS) + "." + CSUB_CONDITIONS_TYPE;
  BSONObj                        q     = BSON(conds << "ONCHANGE");

  if (!collectionQuery(getSubscribeContextCollectionName(tenant), q, &cursor, &err))
  {
    reqSemGive(__FUNCTION__, "Mongo List Subscriptions", reqSemTaken);
    return OrionError(SccReceiverInternalError, err);
  }

  /* Process query result */
  while (cursor->more())
  {
    BSONObj r = cursor->next();
    LM_T(LmtMongo, ("retrieved document: '%s'", r.toString().c_str()));

    Subscription s;
    setSubscriptionId(&s, r);
    setSubject(&s, r);
    setNotification(&s, r);
    setDuration(&s, r);
    subs.push_back(s);
  }

  reqSemGive(__FUNCTION__, "Mongo List Subscriptions", reqSemTaken);
  return OrionError(SccOk);
}
