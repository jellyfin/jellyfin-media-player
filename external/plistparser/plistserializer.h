#pragma once

// Qt includes
#include <QIODevice>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <QDomElement>
#include <QDomDocument>
#include <QString>

class PListSerializer {
public:
	static QString toPList(const QVariant &variant);
private:
	static QDomElement serializeElement(QDomDocument &doc, const QVariant &variant);
	static QDomElement serializeMap(QDomDocument &doc, const QVariantMap &map);
	static QDomElement serializeList(QDomDocument &doc, const QVariantList &list);
};

