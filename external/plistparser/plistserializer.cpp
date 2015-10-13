// Own includes
#include "plistserializer.h"

// Qt includes
#include <QDomElement>
#include <QDomDocument>
#include <QDate>
#include <QDateTime>

static QDomElement textElement(QDomDocument& doc, const char *tagName, QString contents) {
	QDomElement tag = doc.createElement(QString::fromLatin1(tagName));
	tag.appendChild(doc.createTextNode(contents));
	return tag;
}

static QDomElement serializePrimitive(QDomDocument &doc, const QVariant &variant) {
	QDomElement result;
	if (variant.type() == QVariant::Bool) {
        result = doc.createElement(variant.toBool() ? QStringLiteral("true") : QStringLiteral("false"));
	}
	else if (variant.type() == QVariant::Date) {
		result = textElement(doc, "date", variant.toDate().toString(Qt::ISODate));
	}
	else if (variant.type() == QVariant::DateTime) {
		result = textElement(doc, "date", variant.toDateTime().toString(Qt::ISODate));
	}
	else if (variant.type() == QVariant::ByteArray) {
		result = textElement(doc, "data", QString::fromLatin1(variant.toByteArray().toBase64()));
	}
	else if (variant.type() == QVariant::String) {
		result = textElement(doc, "string", variant.toString());
	}
	else if (variant.type() == QVariant::Int) {
		result = textElement(doc, "integer", QString::number(variant.toInt()));
	}
	else if (variant.canConvert(QVariant::Double)) {
		QString num;
		num.setNum(variant.toDouble());
		result = textElement(doc, "real", num);
	}
	return result;
}

QDomElement PListSerializer::serializeElement(QDomDocument &doc, const QVariant &variant) {
	if (variant.type() == QVariant::Map) {
		return serializeMap(doc, variant.toMap());
	}
	else if (variant.type() == QVariant::List) {
		 return serializeList(doc, variant.toList());
	}
	else {
		return serializePrimitive(doc, variant);
	}
}

QDomElement PListSerializer::serializeList(QDomDocument &doc, const QVariantList &list) {
	QDomElement element = doc.createElement(QStringLiteral("array"));
	foreach(QVariant item, list) {
		element.appendChild(serializeElement(doc, item));
	}
	return element;
}

QDomElement PListSerializer::serializeMap(QDomDocument &doc, const QVariantMap &map) {
	QDomElement element = doc.createElement(QStringLiteral("dict"));
	QList<QString> keys = map.keys();
	foreach(QString key, keys) {
		QDomElement keyElement = textElement(doc, "key", key);
		element.appendChild(keyElement);
		element.appendChild(serializeElement(doc, map[key]));
	}
	return element;
}

QString PListSerializer::toPList(const QVariant &variant) {
	QDomDocument document(QStringLiteral("plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\""));
	document.appendChild(document.createProcessingInstruction(QStringLiteral("xml"), QStringLiteral("version=\"1.0\" encoding=\"UTF-8\"")));
	QDomElement plist = document.createElement(QStringLiteral("plist"));
	plist.setAttribute(QStringLiteral("version"), QStringLiteral("1.0"));
	document.appendChild(plist);
	plist.appendChild(serializeElement(document, variant));
	return document.toString();
}
