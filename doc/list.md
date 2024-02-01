# list

## 定义链表

- 链表嵌入结构体(各种宏)
- 初始化结构体成员
  - 动态：INIT_LIST_HEAD
  - 静态：LIST_HEAD_INIT
- 链表头 LIST_HEAD

## 操作链表

- 添加
  - list_add(new, head)
  - list_add_tail(new, head)
- 删除
  - list_del(entry)
- 移动和合并
  - list_move(list, list)
- 替换
  - list_replase

- 遍历
    -
