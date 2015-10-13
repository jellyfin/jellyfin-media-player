#pragma once

// Qt includes
#include <QIODevice>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <QDomElement>

class PListParser {
public:
	static QVariant parsePList(QIODevice *device);
private:
	static QVariant parseElement(const QDomElement &e);
	static QVariantList parseArrayElement(const QDomElement& node);
	static QVariantMap parseDictElement(const QDomElement& element);
};

