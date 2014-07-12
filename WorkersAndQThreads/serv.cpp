#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include <QtDebug>

#include "serv.h"


BaseWorker::~BaseWorker()
{
    emit imDead();
}

void
BaseWorker::connectAll(QThread *, Wrapper * wrapper)
{
    connect(this,SIGNAL(imDead()), wrapper, SLOT(setFlag()));
}

void
ProcessAndFinishWorker::connectAll(QThread * thread, Wrapper * wrapper)
{
    connect(thread, SIGNAL(started()),  this,   SLOT(processAndEmit()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    connect(this,   SIGNAL(finished()), thread, SLOT(quit()));
    connect(this,   SIGNAL(finished()), this,   SLOT(deleteLater()));

    BaseWorker::connectAll(thread, wrapper);
}

void
ProcessAndFinishWorker::processAndEmit()
{
    process();
    emit finished();
}

void
AbortLoopWorker::process()
{
    while( ! isDone()) step();
}

bool
AbortLoopWorker::isDone()
{
    return 1 == m_abort;
}

void
AbortLoopWorker::shutdown()
{
    m_abort.fetchAndStoreOrdered(1);
}

bool
Wrapper::isDone()
{
    return 1 == isWorkerDone;
}

void
Wrapper::setFlag()
{
    isWorkerDone.fetchAndStoreOrdered(1);
}

void
Wrapper::init(BaseWorker * worker)
{
    m_worker = worker;
    isWorkerDone.fetchAndStoreOrdered(0);
    QThread * thread = new QThread();
    m_worker->moveToThread(thread);
    m_worker->connectAll(thread, this);
    thread->start();
}

void
Wrapper::done()
{
    if (isDone()) return;
    m_worker->shutdown();
    while ( ! isDone())
    {
        QCoreApplication::processEvents();
    }
}


ConfigFile::ConfigFile(int aa, int bb, float dd, float ee)
    : a(aa), b(bb), d(dd), e(ee)
{
    w = 3.14159265358979323846 * d;
    z = 1111111111111111111ULL * w;
}

void Config::read(ConfigFile file)
{
    // read foo from file
    foo.a = file.a;
    foo.b = file.b;

    // read far from file
    far.d = file.d;
    far.e = file.e;

    //read faz from file
    faz.w = file.w;
    faz.z = file.z;
}

/* Main Test Class*/
class TestAbortTheLoopWorker : public AbortLoopWorker
{
  public:
    void step()
    {
      qDebug() << "start step";
      std::this_thread::sleep_for(std::chrono::milliseconds(444));
      qDebug() << "stop step";
    }
    void shutdown()
    {
        AbortLoopWorker::shutdown();
        /*break_socket(x);*/
    }
};

void
Foo::init(Config::Foo config)
{
    a = config.a;
    b = config.b;
    m_wrapper.init( new TestAbortTheLoopWorker() );
}

void
Foo::done()
{
    m_wrapper.done();
}
void
Foo::work()
{
    std::cout << "Foo work: {" << a << ", " << b << "}\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(333));
}

void
Far::init(Config::Far config)
{
    d = config.d;
    e = config.e;
}

void
Far::done() {   /*std::cout << "Far: done\n";*/ }
void
Far::work() {
    std::cout << "Far work: {" << d << ", " << e << "}\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(444));
}

void
Faz::init(Config::Faz config)
{
    w = config.w;
    z = config.z;
}

void
Faz::done() {   /*std::cout << "Faz: done\n";*/ }
void
Faz::work()
{
    std::cout << "Faz work: {" << w << ", " << z << "}\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(777));
}

void
Plugin::init(ConfigFile file)
{
    Config config;
    config.read(file);

    m_foo.init(config.foo);
    m_far.init(config.far);
    m_faz.init(config.faz);
}

void
Plugin::reload(ConfigFile file)
{
    done();
    qDebug() << "Plugin::reload, let's sleep a bit";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    qDebug() << "Plugin::reload, wake up!";

    init(file);
}

void
Plugin::done()
{
    m_faz.done();
    m_far.done();
    m_foo.done();
}

void
Plugin::work()
{
    m_foo.work();
    m_far.work();
    m_faz.work();
}


