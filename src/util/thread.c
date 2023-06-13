#include "thread.h"

// 动态调整线程池大小可以根据当前任务的数量和系统负载情况来自动调整线程池中的工作线程数量，以达到更好的性能和资源利用率。一般而言，可以采用以下两种方式来实现线程池的动态调整：

// 基于任务数量
// 当任务数量增加时，可以动态地向线程池中添加新的工作线程，以便更快地处理任务。反之，当任务数量减少时，可以动态地销毁空闲的工作线程，以节省资源。

// 具体而言，可以在添加或取消任务时，检查当前线程池中的工作线程数量是否符合要求。例如，如果当前线程池中的工作线程数目小于指定的最小线程数目，那么就需要创建新的线程；如果当前线程池中的工作线程数目大于指定的最大线程数目，那么就需要停止部分线程，以避免过度消耗系统资源。

// 基于系统负载
// 当系统负载较高时，可以动态地降低线程池中的工作线程数量，以减少系统开销和竞争情况。反之，当系统负载较低时，可以动态地增加线程池中的工作线程数量，以更快地处理任务和提高并行性能。

// 具体而言，可以通过监控系统负载、CPU 使用率、IO
// 等指标来判断当前系统的负载情况。例如，在 Linux 系统中，可以使用 sysstat
// 工具包中的 sar
// 命令来实时监控系统负载情况。如果发现系统负载较高，就需要动态地降低线程池中的工作线程数量；反之，如果发现系统负载较低，就可以动态地增加线程池中的工作线程数量。不过需要注意的是，系统负载不一定与任务数量成正比，因此需要根据实际情况进行调整。

// 需要注意的是，在动态调整线程池大小时，需要采用互斥锁等同步机制来保护线程池中的共享资源，以避免竞争和资源泄漏等问题。