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
  explicit CachedRegexMatcher(bool allowMultiplePatterns = true, QObject* parent = nullptr)
    : QObject(parent), m_allowMultiplePatterns(allowMultiplePatterns) {}

  bool addMatcher(const QString& pattern, const QVariant& result);
  QVariantList match(const QString& input);
  void clear();

private:
  MatcherValueList m_matcherList;
  QHash<QString, QVariantList> m_matcherCache;
  bool m_allowMultiplePatterns;
};

#endif //KONVERGO_CACHEDREGEXMATCHER_H
