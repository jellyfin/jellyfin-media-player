//
// Created by Tobias Hieta on 20/08/15.
//

#ifndef KONVERGO_CACHEDREGEXMATCHER_H
#define KONVERGO_CACHEDREGEXMATCHER_H

#include <QRegExp>
#include <QVariant>
#include <QString>
#include <QHash>

typedef QPair<QRegExp, QVariant> MatcherValuePair;
typedef QList<MatcherValuePair> MatcherValueList;

class CachedRegexMatcher : public QObject
{
public:
  explicit CachedRegexMatcher(QObject* parent = nullptr) : QObject(parent) {}

  bool addMatcher(const QString& pattern, const QVariant& result);
  QVariant match(const QString& input);
  void clear();

private:
  MatcherValueList m_matcherList;
  QHash<QString, QVariant> m_matcherCache;
};

#endif //KONVERGO_CACHEDREGEXMATCHER_H
