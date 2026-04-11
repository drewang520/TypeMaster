#ifndef IOBSERVER_H
#define IOBSERVER_H
 
/**
 * IObserver — 观察者模式的纯接口。
 * 任何希望接收模型变更通知的类
 * 都必须继承自该接口并实现 onUpdate() 方法。
 */
class IObserver
{
public:
    virtual ~IObserver() = default;
 
    /**
     * 每当模型状态发生变化时，由 GameData::notifyObservers() 调用。
     * 实现类应刷新其显示效果或触发重绘。
     */
    virtual void onUpdate() = 0;
};

#endif // IOBSERVER_H