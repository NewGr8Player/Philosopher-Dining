# 哲学家就餐问题(预先分配解决死锁问题)

**使用Visual Stdio 2012或以上版本运行**

```
菜单项目："预先分配演示",      IDM_WAITFORMULTIPLE
菜单项目："快速死锁演示",      IDM_FASTDEADLOCK
菜单项目："普通死锁演示",      IDM_SLOWDEADLOCK
菜单项目："结束所有线程",      IDM_TERMINATETHREAD
菜单项目："创建工作线程",      IDM_CREATETHREAD
菜单项目："挂起所有线程",      IDM_SUSPENDTHREAD
菜单项目："恢复所有进程",      IDM_RESUMETHREAD
```

> "预先分配演示","快速死锁演示","普通死锁演示"： 切换前最好结束线程

> "快速死锁演示","普通死锁演示" ：调用函数基本相同，休眠时间长短不同
