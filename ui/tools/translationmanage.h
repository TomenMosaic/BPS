#ifndef TRANSLATIONMANAGE_H
#define TRANSLATIONMANAGE_H

#include "database.h"
#include <QMap>
#include <QObject>
#include "GlobalVar.h"
class TranslationManage : public QObject
{
    Q_OBJECT

public:
    enum LanguageType
    {
        Chinese,
        English
    };
    QString getTranslation(RealDataType dataType);
    QString getUnit(RealDataType dataType);
    QString getTranslation(FlowType flowType);
    QString getTranslation(Cavity cavity);
    QString getTranslation(SampleTypes sampleType);

    bool comtain(LanguageType type,QLocale locale);
    static TranslationManage *getInstance(QObject *parent = nullptr);
    static TranslationManage *pTrans;
    void init();

    QStringList getTranslations(QList<int>idList);

    QString getTranslation(int id);

    QIcon getTranslationPic(int id);

    //会根据子组件的objectName，自动设置子组件的名称
    void autoTransLate(QObject * parent);

    LanguageType getLanguage() const;

    QLocale getLanguageStr(LanguageType type);

    QString languagefromlocale(QLocale locale);

    void setLanguage(QLocale newLanguage);

    void setLanguageType(LanguageType newLanguage);

private:
    //获取组件的子组件列表
    QObjectList getChildList(QObject *obj);
    TranslationManage(QObject *parent = nullptr);
signals:


private:
    LanguageType language = English;
    QMap<int,QStringList>mapList;
};

#endif // TRANSLATIONMANAGE_H
