#ifndef XXX
#define XXX

#include <QObject>
#include <QAtomicInt>
#include <QThread>
#include <QCoreApplication>

class Wrapper;

class BaseWorker : public QObject
{
  Q_OBJECT
  friend class Wrapper;
  public:
    BaseWorker() {}
    virtual ~BaseWorker();
  protected:
    virtual void connectAll(QThread*, Wrapper*);
    virtual void shutdown() = 0;
  signals:
    void imDead();
};

class ProcessAndFinishWorker : public BaseWorker
{
  Q_OBJECT
  public:
    ProcessAndFinishWorker()  {}
    ~ProcessAndFinishWorker() {}

  protected:
    virtual void process() = 0;
    void connectAll(QThread*, Wrapper*);

  signals:
    void finished();

  private slots:
    void processAndEmit();
};

class AbortLoopWorker : public ProcessAndFinishWorker
{
  Q_OBJECT
  public slots:
    void process();

  protected:
    virtual void step() = 0;
    void shutdown();

  private:
    bool isDone();
    QAtomicInt m_abort;
};

class QueuedWorker : public ProcessAndFinishWorker
{

};

class RandomActiveWorker : public BaseWorker
{

};

class TimerWorker : public RandomActiveWorker
{

};

class ScheduledWorker : public RandomActiveWorker
{

};

class CallbackWorker : public RandomActiveWorker
{

};

class Wrapper : public QObject
{
  Q_OBJECT
  public:
    bool isDone();
    void init(BaseWorker * worker);
    void done();
  public slots:
    void setFlag();

  private:
    BaseWorker * m_worker;
    QAtomicInt isWorkerDone;
};












class ConfigFile
{
   public:
       ConfigFile(int aa, int bb, float dd, float ee);
       int    a, b;
       float  d, e;
       double w, z;
};

struct Config
{
    struct Foo { int    a, b; } foo;
    struct Far { float  d, e; } far;
    struct Faz { double w, z; } faz;
    void read(ConfigFile file);
};

class Foo
{
    public:
        void init(Config::Foo config);
        void reload();  /* NO NEED FOR THIS!!! */
        void done();
        void work();
    private:
        int a, b;
        Wrapper m_wrapper;  /* wrapper остаётся жить при внешнем reload! */
};

class Far
{
    public:
        void init(Config::Far config);
        void reload();  /* NO NEED FOR THIS!!! */
        void done();
        void work();
    private:
        float d, e;
};

class Faz
{
    public:
        void init(Config::Faz config);
        void reload();  /* NO NEED FOR THIS!!! */
        void done();
        void work();
    private:
        float w, z;
};

class Plugin
{
  public:
    void init(ConfigFile file);
    void reload(ConfigFile file);
    void done();
    void work();
  private:
    Foo m_foo;
    Far m_far;
    Faz m_faz;
};

#endif // XXX
