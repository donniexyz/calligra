#ifndef KWORD_FACTORY_H
#define KWORD_FACTORY_H

#include <klibloader.h>

class KWordFactory : public KLibFactory
{
    Q_OBJECT
public:
    KWordFactory( QObject* parent = 0, const char* name = 0 );
    ~KWordFactory();

    virtual QObject* create( QObject* parent = 0, const char* name = 0, const char* classname = "QObject", const QStringList &args = QStringList() );

    static KInstance* global();

private:
    static KInstance* s_global;
};

#endif
