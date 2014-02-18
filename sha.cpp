#include <QCryptographicHash>
#include <QXmlStreamWriter>
#include <QDir>

using ItemListType = QVector<QPair<QString, QString>>;

QString
sha1(const QString& filepath) {
    QFile file{filepath};
    if (file.open(QIODevice::ReadOnly)) {
        auto fileData = file.readAll();
        return QCryptographicHash::hash(fileData, QCryptographicHash::Sha1).toHex();
    }
    return {};
}

void
writeXml(const QString& filepath, const ItemListType& itemlist) {
    QFile file{filepath};
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {  // QIODevice::Text for CRLF (\r\n)
        QXmlStreamWriter w{&file};
        w.setAutoFormatting(true);
        w.setAutoFormattingIndent(2);
        w.writeStartDocument();
        w.writeStartElement(QStringLiteral("items"));
          for (const auto& pair : itemlist) {
            w.writeStartElement(QStringLiteral("item"));
              w.writeTextElement(QStringLiteral("filename"), pair.first);
              w.writeTextElement(QStringLiteral("sha1"),     pair.second);
            w.writeEndElement();
          }
        w.writeEndElement();
        w.writeEndDocument();
    }
}

int
main()
{
    auto fpddir   = QDir{QStringLiteral("/tmp")};
    auto filter   = QStringList{QStringLiteral("*.xml")};
    auto infolist = fpddir.entryInfoList(filter, QDir::Files);
    auto itemlist = ItemListType{};

    for (const auto& info : infolist) {
        auto full = info.canonicalFilePath();
        auto pair = qMakePair(info.fileName(), sha1(full));
        writeXml(full + QStringLiteral(".sha1"), ItemListType{pair});
        itemlist.append(pair);
    }

    auto all = fpddir.absolutePath() + QStringLiteral("\\all");
    writeXml(all, itemlist);

    auto pair = qMakePair(QStringLiteral("all"),sha1(all));
    writeXml(all + QStringLiteral(".sha1"), ItemListType{pair});
}
