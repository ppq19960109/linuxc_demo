/***********************************************************
*文件名     : link_list.c
*版   本   : v1.0.0.0
*日   期   : 2018.05.31
*说   明   : 双向循环链表类相关接口
*修改记录: 
************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "scene_link_list.h"
#include "param_check.h"

/*在当前结点后插入新结点*/
#define INSERT_NODE(curNode, insNode)			\
do{            									\
	(insNode)->pNext		= (curNode)->pNext;	\
	(insNode)->pPrev		= (curNode);		\
	(curNode)->pNext->pPrev	= (insNode);		\
	(curNode)->pNext		= (insNode);		\
}while(0)
//删除链表结点
#define REMOVE_NODE(rmNode)						\
do{												\
	(rmNode)->pPrev->pNext = (rmNode)->pNext;	\
	(rmNode)->pNext->pPrev = (rmNode)->pPrev;	\
}while(0)

#pragma pack(1)

/*私有类型定义*/
typedef struct link_list_private_s  
{  
	int iReadLock;
	int iWriteLock;
}link_list_private_t;  
#pragma pack()

/*************************************************************
*方法:	link_list_read_lock
*参数:	_this:类指针
*返回值:0表示成功，非0表示失败
*描述:	获取读锁（该接口为阻塞接口）
*************************************************************/
static int	
link_list_read_lock(link_list_class_t * _this)
{
	return NoErr;
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	link_list_private_t *pstPtr = 
		(link_list_private_t*)(_this->acPrivateParam);
	while(1)
	{
		/*只要写锁未加锁即可成功获取读锁*/
		if(!pstPtr->iWriteLock)
		{
			(pstPtr->iReadLock)++;
			break;
		}
		else
		{
			/*获取读锁失败,等待再次尝试*/
			HY_DEBUG("read lock failed\n");
			usleep(100);
		}
	}

	return NoErr;
}

/*************************************************************
*方法:	link_list_read_lock_non_blocking
*参数:	_this:类指针
*返回值:0表示成功，非0表示失败
*描述:	获取读锁（该接口为非阻塞接口）
*************************************************************/
static int	
link_list_read_lock_non_blocking(link_list_class_t * _this)
{
	return NoErr;
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	link_list_private_t *pstPtr = 
		(link_list_private_t*)(_this->acPrivateParam);
	/*只要写锁未加锁即可成功获取读锁*/
	if(!pstPtr->iWriteLock)
	{
		(pstPtr->iReadLock)++;
		return NoErr;
	}
	else
	{
		/*获取读锁失败,等待再次尝试*/
		HY_DEBUG("read lock failed\n");
		return MutexLockErr;
	}
}

/*************************************************************
*方法:	link_list_read_unlock
*参数:	_this:类指针
*返回值:0表示成功，非0表示失败
*描述:	释放读锁
*************************************************************/
static int	
link_list_read_unlock(link_list_class_t *_this)
{
	return NoErr;
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	link_list_private_t *pstPtr = 
		(link_list_private_t*)(_this->acPrivateParam);
	if(pstPtr->iReadLock)
	{
		pstPtr->iReadLock--;
	}

	return NoErr;
}

/*************************************************************
*方法:	link_list_write_lock
*参数:	_this:类指针
*返回值:0表示成功，非0表示失败
*描述:	获取写锁（该接口为阻塞接口）
*************************************************************/
static int	
link_list_write_lock(link_list_class_t *_this)
{
	return NoErr;
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	link_list_private_t *pstPtr = 
		(link_list_private_t*)(_this->acPrivateParam);
	/*只要读锁加锁或写锁加锁，则获取读锁失败*/
	if(pstPtr->iReadLock ||
		pstPtr->iWriteLock)
	{
		/*获取写锁失败,等待再次尝试*/
		HY_DEBUG("write lock failed\n");
		return MutexLockErr;
	}
	else
	{
		(pstPtr->iWriteLock)++;
		return NoErr;
	}

}

/*************************************************************
*方法:	link_list_write_lock_non_blocking
*参数:	_this:类指针
*返回值:0表示成功，非0表示失败
*描述:	获取写锁（该接口为非阻塞接口）
*************************************************************/
static int	
link_list_write_lock_non_blocking(link_list_class_t *_this)
{
	return NoErr;
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	link_list_private_t *pstPtr = 
		(link_list_private_t*)(_this->acPrivateParam);
	/*只要读锁加锁或写锁加锁，则获取读锁失败*/
	if(pstPtr->iReadLock ||
		pstPtr->iWriteLock)
	{
		/*获取写锁失败,等待再次尝试*/
		HY_DEBUG("write lock failed\n");
		return MutexLockErr;
	}
	else
	{
		(pstPtr->iWriteLock)++;
		return NoErr;
	}

}

/*************************************************************
*方法:	link_list_write_unlock
*参数:	类指针
*返回值:0表示成功，非0表示失败
*描述:	释放写锁
*************************************************************/
static int	
link_list_write_unlock(link_list_class_t *_this)
{
	return NoErr;
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	link_list_private_t *pstPtr = 
		(link_list_private_t*)(_this->acPrivateParam);
	if(pstPtr->iWriteLock)
	{
		pstPtr->iWriteLock--;
	}

	return NoErr;
}

/*************************************************************
*方法:	link_list_size
*参数:	_this:类指针
*返回值:>=0表示链表元素个数，<0表示失败
*描述:	获取链表元素个数
*************************************************************/
static int 
link_list_size(link_list_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	int iListSize = 0;
	/*获取读锁*/
	_this->read_lock(_this);
	iListSize = _this->iNodeNum;
	/*释放读锁*/
	_this->read_unlock(_this);
	
	return iListSize;
}
/*************************************************************
*方法:	link_list_clear
*参数:	_this:类指针
*返回值:0表示成功，非0表示失败
*描述:	清空链表
*************************************************************/
static int 
link_list_clear(link_list_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);

	while(NoErr == _this->del_head(_this));
	return NoErr;
}
/*************************************************************
*方法:	link_list_get_next
*参数:	_this:类指针
*		pstCurrent:当前连接件
*返回值:成功返回下一个连接件，失败返回NULL
*描述:	获取下一个连接件，如果当前连接件为NULL，则返回头结点的next
*************************************************************/
static link_list_piece_t* 
link_list_get_next(link_list_class_t *_this, 
					link_list_piece_t *pstCurrent)
{
	PARAM_CHECK_RETURN_NULL_1(_this);
	link_list_piece_t *pstPtr = NULL;
	if(NULL == pstCurrent)
	{
		/*返回头结点的next*/
		pstPtr = _this->first(_this);
	}
	else
	{
		/*返回next*/
		pstPtr = pstCurrent->pNext;
	}

	return pstPtr;
}
/*************************************************************
*方法:	link_list_get_prev
*参数:	_this:类指针
*		pstCurrent:当前连接件
*返回值:成功返回前一个连接件，失败返回NULL
*描述:	获取前一个连接件，如果当前连接件为NULL，则返回尾结点
*************************************************************/
static link_list_piece_t* 
link_list_get_prev(link_list_class_t *_this, 
					link_list_piece_t *pstCurrent)
{
	PARAM_CHECK_RETURN_NULL_1(_this);
	link_list_piece_t *pstPtr = NULL;
	if(NULL == pstCurrent)
	{
		/*返回尾结点*/
		pstPtr = _this->tail(_this);
	}
	else
	{
		/*返回prev*/
		pstPtr = pstCurrent->pPrev;
	}

	return pstPtr;
}
/*************************************************************
*方法:	link_list_new_node
*参数:	_this:类指针
*返回值:成功返回连接件，失败返回NULL
*描述:	新建一个连接件
*************************************************************/
static link_list_piece_t* 
link_list_new_node(link_list_class_t *_this)
{
	PARAM_CHECK_RETURN_NULL_1(_this);
	link_list_piece_t *pstPtr = 
		(link_list_piece_t *)calloc(1, _this->iNodeDataSize);
	if(NULL == pstPtr)
	{
		HY_ERROR("Malloc error: %s\n", strerror(errno));
		return NULL;
	}
	
	return pstPtr;
}
/*************************************************************
*方法:	link_list_destroy_node
*参数:	_this:类指针
*		pstCurrent:要销毁的连接件
*返回值:0表示成功，非0表示失败
*描述:	新建一个连接件
*************************************************************/
static int 
link_list_destroy_node(link_list_class_t *_this, 
							link_list_piece_t *pstCurrent)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstCurrent);

	free(pstCurrent);
	pstCurrent = NULL;

	return NoErr;
}
/*************************************************************
*方法:	link_list_inst_head
*参数:	类指针
*		要插入的链接件
*返回值:0表示成功，非0表示失败
*描述:	在pHead后插入连接件
*************************************************************/
static int 
link_list_inst_head(link_list_class_t *_this, 
						link_list_piece_t *pstInstValue)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstInstValue);
	return _this->inst(_this, _this->head(_this), pstInstValue);
}
/*************************************************************
*方法:	link_list_inst_tail
*参数:	类指针
*		要插入的链接件
*返回值:0表示成功，非0表示失败
*描述:	在pTail后插入连接件
*************************************************************/
static int 
link_list_inst_tail(link_list_class_t *_this,
						link_list_piece_t *pstInstValue)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstInstValue);
	return _this->inst(_this, _this->tail(_this), pstInstValue);
}

/*************************************************************
*方法:	link_list_inst_by_id
*参数:	类指针
*		iId位置序号
*		要插入的连接件
*返回值:0表示成功，非0表示失败
*描述:	在指定位置序号后插入连接件
*************************************************************/
static int 
link_list_inst_by_id(link_list_class_t *_this, 
						int iId, 
						link_list_piece_t *pstInstValue)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstInstValue);
	PARAM_CHECK_NEGATIVE_NUMBER_RETURN_ERRORNO_1(iId);

	return _this->inst(_this, _this->get_by_id(_this, iId), pstInstValue);
	
}
/*************************************************************
*方法:	link_list_inst
*参数:	_this:类指针
*		pstCurrent:位置
*		pstInstValue:要插入的连接件
*返回值:0表示成功，非0表示失败
*描述:	在指定位置后插入连接件
*************************************************************/
static int 
link_list_inst(link_list_class_t *_this, 
				link_list_piece_t *pstCurrent,
				link_list_piece_t *pstInstValue)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pstCurrent, pstInstValue);
	/*获取写锁*/
	_this->write_lock(_this);
	//DEBUG("link_list_inst pstCurrent = %p\n", pstCurrent);
	/*将结点插在当前结点后，如果当前结点为尾结点，则需要更改尾结点的值*/
	INSERT_NODE(pstCurrent, pstInstValue);
	if(_this->pTail == pstCurrent)
	{
		_this->pTail = pstInstValue;
	}
	_this->iNodeNum ++;
	/*释放写锁*/
	_this->write_unlock(_this);
	
	return NoErr;
}

/*************************************************************
*方法:	link_list_del_head
*参数:	_this:类指针
*返回值:成功返回连接件，失败返回NULL
*描述:	删除头结点的next
*************************************************************/
static int 
link_list_del_head(link_list_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	return _this->destroy_node(_this, _this->del_head_result(_this));
}
/*************************************************************
*方法:	link_list_del_head_result
*参数:	_this:类指针
*返回值:0表示成功，非0表示失败
*描述:	删除头结点的next(只断开连接件的链接，连接件以指针方式返回)
*************************************************************/
static link_list_piece_t* 
link_list_del_head_result(link_list_class_t *_this)
{
	PARAM_CHECK_RETURN_NULL_1(_this);
	return _this->del_result(_this, _this->first(_this));
}

/*************************************************************
*方法:	link_list_del_tail
*参数:	_this:类指针
*返回值:0表示成功，非0表示失败
*描述:	删除尾结点
*************************************************************/
static int 
link_list_del_tail(link_list_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	return _this->destroy_node(_this, _this->del_tail_result(_this));
}
/*************************************************************
*方法:	link_list_del_tail_result
*参数:	_this:类指针
*返回值:成功返回连接件，失败返回NULL
*描述:	删除尾结点(只断开连接件的链接，连接件以指针方式返回)
*************************************************************/
static link_list_piece_t* 
link_list_del_tail_result(link_list_class_t *_this)
{
	PARAM_CHECK_RETURN_NULL_1(_this);
	return _this->del_result(_this, _this->tail(_this));
}
/*************************************************************
*方法:	link_list_del
*参数:	_this:类指针
*		pstCurrent:要删除的连接件
*返回值:0表示成功，非0表示失败
*描述:	删除指定结点
*************************************************************/
static int 
link_list_del(link_list_class_t *_this, 
				link_list_piece_t *pstCurrent)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstCurrent);
	return _this->destroy_node(_this, _this->del_result(_this, pstCurrent));
}
/*************************************************************
*方法:	link_list_del_result
*参数:	_this:类指针
*		pstCurrent:要删除的连接件
*返回值:成功返回连接件，失败返回NULL
*描述:	删除指定结点(只断开连接件的链接，连接件以指针方式返回)
*************************************************************/
static link_list_piece_t* 
link_list_del_result(link_list_class_t *_this, 
						link_list_piece_t *pstCurrent)
{
	PARAM_CHECK_RETURN_NULL_2(_this, pstCurrent);
	/*如果该节点为头结点，则不允许删除*/
	if(pstCurrent == _this->head(_this))
	{
		return NULL;
	}
	/*获取写锁*/
	_this->write_lock(_this);
	/*删除尾结点，更改尾结点的值*/
	REMOVE_NODE(pstCurrent);
	if(pstCurrent == _this->pTail)
	{
		_this->pTail = pstCurrent->pPrev;
	}
	_this->iNodeNum --;
	/*释放写锁*/
	_this->write_unlock(_this);

	return pstCurrent;
}
/*************************************************************
*方法:	link_list_set_by_id
*参数:	_this:类指针
*		iId:被设置值的ID序号
*		pstSetValue:设置值
*返回值:0表示成功，非0表示失败
*描述:	设置指定id的值
*************************************************************/
static int 
link_list_set_by_id(link_list_class_t *_this, 
					int iId, 
					link_list_piece_t *pstSetValue)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstSetValue);
	PARAM_CHECK_NEGATIVE_NUMBER_RETURN_ERRORNO_1(iId);
	return _this->set(_this, _this->get_by_id(_this, iId), pstSetValue);
}
/*************************************************************
*方法:	link_list_set
*参数:	_this:类指针
*		pstCurrent:被设置值
*		pstSetValue:设置值
*返回值:0表示成功，非0表示失败
*描述:	设置指定结点的值
*************************************************************/
static int 
link_list_set(link_list_class_t *_this,
				link_list_piece_t *pstCurrent,
				link_list_piece_t *pstSetValue)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, pstCurrent, pstSetValue);
	/*获取当前结点的前一个结点*/
	link_list_piece_t *pstPrev = _this->prev(_this, pstCurrent);
	/*删除当前结点*/
	_this->del(_this,pstCurrent);
	/*在pstPrev后，插入新结点*/
	_this->inst(_this, pstPrev, pstSetValue);
	
	return NoErr;
}

/*************************************************************
*方法:	link_list_get
*参数:	_this:类指针
*		pCompareHandle:比较函数
*		pstCmpParam:比较函数参数
*返回值:0表示成功，非0表示失败
*描述:	获取指定下标的链接件
*************************************************************/
static link_list_piece_t* 
link_list_get(link_list_class_t *_this, 
				ComparePieceFunc pCompareHandle,
				link_list_piece_t *pstCmpParam)
{
	PARAM_CHECK_RETURN_NULL_3(_this, pCompareHandle, pstCmpParam);
	
	link_list_piece_t *pstPtr = _this->next(_this, NULL);
	if(NULL == pstPtr)
	{
		return NULL;
	}
	link_list_piece_t *pstHead = _this->head(_this);
	if(NULL == pstHead)
	{
		return NULL;
	}
	while(pstPtr && pstHead != pstPtr)
	{
		
		if(!pCompareHandle(pstPtr, pstCmpParam))
		{
			return pstPtr;
		}
		pstPtr = _this->next(_this, pstPtr);
	}
	return NULL;
}
/*************************************************************
*方法:	link_list_get_by_id
*参数:	_this:类指针
*		iId:要获取的连接件的id序号
*返回值:0表示成功，非0表示失败
*描述:	获取指定下标的链接件
*************************************************************/
static link_list_piece_t* 
link_list_get_by_id(link_list_class_t *_this,int iId)
{
	PARAM_CHECK_RETURN_NULL_1(_this);
	PARAM_CHECK_NEGATIVE_NUMBER_RETURN_NULL_1(iId);
	if(iId > _this->size(_this))
	{
		return NULL;
	}

	int iCount = 0;
	link_list_piece_t *pstPtr = _this->next(_this, NULL);
	if(NULL == pstPtr)
	{
		return NULL;
	}
	link_list_piece_t *pstHead = _this->head(_this);
	if(NULL == pstHead)
	{
		return NULL;
	}
	while(pstHead != pstPtr)
	{
		if(iId == iCount)
		{
			return pstPtr;
		}
		iCount++;
		pstPtr = _this->next(_this, pstPtr);
	}
	return NULL;
}

/*************************************************************
*方法:	link_list_get_index
*参数:	_this:类指针
*		pstCurrent:要获取的连接件的指针
*返回值:<0表示失败，0>=表示连接件的下标
*描述:	获取链接件的下标
*************************************************************/
static int 
link_list_get_index(link_list_class_t *_this, link_list_piece_t *pstCurrent)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pstCurrent);

	int iCount = 0;
	link_list_piece_t *pstPtr = _this->next(_this, NULL);
	if(NULL == pstPtr)
	{
		return SceneGetErr;
	}
	link_list_piece_t *pstHead = _this->head(_this);
	if(NULL == pstHead)
	{
		return SceneGetErr;
	}
	while(pstHead != pstPtr)
	{
		if(pstCurrent == pstPtr)
		{
			return iCount;
		}
		iCount++;
		pstPtr = _this->next(_this, pstPtr);
	}
	return NotFoundErr;
}

/*************************************************************
*方法:	link_list_get_head
*参数:	_this:类指针
*返回值:成功返回连接件，失败返回NULL
*描述:	删除头结点
*************************************************************/
static link_list_piece_t* 
link_list_get_head(link_list_class_t *_this)
{
	PARAM_CHECK_RETURN_NULL_1(_this);
	
	return _this->pHead;
}


/*************************************************************
*方法:	link_list_get_first
*参数:	_this:类指针
*返回值:成功返回连接件，失败返回NULL
*描述:	删除头结点的next
*************************************************************/
static link_list_piece_t* 
link_list_get_first(link_list_class_t *_this)
{
	PARAM_CHECK_RETURN_NULL_1(_this);
	//DEBUG("size = %d\n", _this->size(_this));
	//DEBUG("head = %p, tail = %p, head next = %p\n", _this->pHead, _this->pTail, _this->pHead->pNext);
	if(_this->size(_this))
	{
		return _this->pHead->pNext;
	}
	else
	{
		return NULL;
	}
}
/*************************************************************
*方法:	link_list_get_tail
*参数:	_this:类指针
*返回值:成功返回连接件，失败返回NULL
*描述:	获取尾结点
*************************************************************/
static link_list_piece_t* 
link_list_get_tail(link_list_class_t *_this)
{
	PARAM_CHECK_RETURN_NULL_1(_this);

	return _this->pTail;
}


/*************************************************************
*函数:	link_list_traverse
*参数:	_this:类指针
*		pTraverseHandle:遍历操作函数
*		pstTraverseParam:遍历操作函数参数
*返回值:0表示成功，非0表示失败
*描述:	遍历链表，每个节点数据进行TravNodeFunc指定的操作
*************************************************************/
static int 
link_list_traverse(link_list_class_t *_this,
					TravPieceFunc pTraverseHandle, 
					link_list_piece_t *pstTraverseParam)
{
	PARAM_CHECK_RETURN_ERRORNO_2(_this, pTraverseHandle);

	int iRet = 0;
	link_list_piece_t *pstPtr = _this->next(_this, NULL);
	if(NULL == pstPtr)
	{
		return SceneGetErr;
	}
	link_list_piece_t *pstHead = _this->head(_this);
	if(NULL == pstHead)
	{
		return SceneGetErr;
	}
	while(pstHead != pstPtr)
	{
		iRet = pTraverseHandle(pstPtr, pstTraverseParam);
		if(NoErr != iRet)
		{
			HY_ERROR("Traverse failed.\n");
		}
		pstPtr = _this->next(_this, pstPtr);
	}

	return iRet;
}
/*************************************************************
*方法:	link_list_print
*参数:	_this:类指针
*		pPrintHandle:打印函数
*返回值:0表示成功，非0表示失败
*描述:	打印链表数据
*************************************************************/
static int 
link_list_print(link_list_class_t *_this, 
				PrintPieceFunc pPrintHandle)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	link_list_piece_t *pstPtr = _this->next(_this, NULL);
	if(NULL == pstPtr)
	{
		return SceneGetErr;
	}
	link_list_piece_t *pstHead = _this->head(_this);
	if(NULL == pstHead)
	{
		return SceneGetErr;
	}
	HY_DEBUG("Count = %d\n", _this->iNodeNum);
	while(pstHead != pstPtr)
	{
		pPrintHandle(pstPtr);
		pstPtr = _this->next(_this, pstPtr);
	}
	return NoErr;
}




/*构造函数*/
link_list_class_t *new_link_list(int iNodeDataSize)
{
	PARAM_CHECK_NEGATIVE_NUMBER_RETURN_NULL_1(iNodeDataSize);
	
	link_list_class_t* pstLinkList = 
		(link_list_class_t *)calloc(1, sizeof(link_list_class_t));
	if(NULL == pstLinkList)
	{
		HY_ERROR("Malloc error: %s\n", strerror(errno));
		return NULL;
	}


	/*参数初始化*/
	/*私有变量初始化*/
	link_list_private_t *pPrivate = (link_list_private_t *)pstLinkList->acPrivateParam;
	memset(pPrivate, 0x0, sizeof(link_list_private_t));
	/*创建头结点*/
	link_list_piece_t *pstPtr = 
		(link_list_piece_t *)calloc(1, iNodeDataSize);
	if(NULL == pstPtr)
	{
		HY_ERROR("Malloc error: %s\n", strerror(errno));
		free(pstLinkList);
		return NULL;
	}
	
	pstPtr->pNext = pstPtr->pPrev = pstPtr;
	pstLinkList->pHead = pstLinkList->pTail = pstPtr;
	
	pstLinkList->iNodeNum = 0;
	
	pstLinkList->iNodeDataSize = iNodeDataSize;
	/*初始化函数指针*/
	pstLinkList->read_lock = link_list_read_lock;
	pstLinkList->read_lock_non_blocking = link_list_read_lock_non_blocking;
	pstLinkList->read_unlock = link_list_read_unlock;
	pstLinkList->write_lock = link_list_write_lock;
	pstLinkList->write_lock_non_blocking = link_list_write_lock_non_blocking;
	pstLinkList->write_unlock = link_list_write_unlock;
	pstLinkList->size = link_list_size;
	pstLinkList->clear = link_list_clear;
	pstLinkList->next = link_list_get_next;
	pstLinkList->prev = link_list_get_prev;
	pstLinkList->new_node = link_list_new_node;
	pstLinkList->destroy_node = link_list_destroy_node;
	pstLinkList->inst_head = link_list_inst_head;
	pstLinkList->inst_tail = link_list_inst_tail;
	pstLinkList->inst = link_list_inst;
	pstLinkList->inst_by_id = link_list_inst_by_id;
	pstLinkList->del_head = link_list_del_head;
	pstLinkList->del_head_result = link_list_del_head_result;
	pstLinkList->del_tail = link_list_del_tail;
	pstLinkList->del_tail_result = link_list_del_tail_result;
	pstLinkList->del = link_list_del;
	pstLinkList->del_result = link_list_del_result;
	pstLinkList->set = link_list_set;
	pstLinkList->set_by_id = link_list_set_by_id;
	pstLinkList->get = link_list_get;
	pstLinkList->get_by_id = link_list_get_by_id;
	pstLinkList->get_index = link_list_get_index;
	pstLinkList->head = link_list_get_head;
	pstLinkList->first = link_list_get_first;
	pstLinkList->tail = link_list_get_tail;
	pstLinkList->traverse = link_list_traverse;
	pstLinkList->print = link_list_print;
/*	
	DEBUG("read_lock  = %p\n", pstLinkList->read_lock);
	DEBUG("read_lock_non_blocking = %p\n", pstLinkList->read_lock_non_blocking);
	DEBUG("read_unlock = %p\n", pstLinkList->read_unlock);
	DEBUG("write_lock = %p\n", pstLinkList->write_lock);
	DEBUG("write_lock_non_blocking = %p\n", pstLinkList->write_lock_non_blocking);
	DEBUG("write_unlock = %p\n", pstLinkList->write_unlock);
	DEBUG("size = %p\n", pstLinkList->size);
	DEBUG("clear = %p\n", pstLinkList->clear);
	DEBUG("next = %p\n", pstLinkList->next);
	DEBUG("prev = %p\n", pstLinkList->prev);
	DEBUG("new_node = %p\n", pstLinkList->new_node);
	DEBUG("destroy_node = %p\n", pstLinkList->destroy_node);
	DEBUG("inst_head = %p\n", pstLinkList->inst_head);
	DEBUG("inst_tail = %p\n", pstLinkList->inst_tail);
	DEBUG("inst = %p\n", pstLinkList->inst);
	DEBUG("inst_by_id = %p\n", pstLinkList->inst_by_id);
	DEBUG("del_head = %p\n", pstLinkList->del_head);
	DEBUG("del_head_result = %p\n", pstLinkList->del_head_result);
	DEBUG("del_tail = %p\n", pstLinkList->del_tail);
	DEBUG("del_tail_result = %p\n", pstLinkList->del_tail_result);
	DEBUG("del = %p\n", pstLinkList->del);
	DEBUG("del_result = %p\n", pstLinkList->del_result);
	DEBUG("set = %p\n", pstLinkList->set);
	DEBUG("set_by_id = %p\n", pstLinkList->set_by_id);
	DEBUG("get = %p\n", pstLinkList->get);
	DEBUG("get_by_id = %p\n", pstLinkList->get_by_id);
	DEBUG("head = %p\n", pstLinkList->head);
	DEBUG("first = %p\n", pstLinkList->first);
	DEBUG("tail = %p\n", pstLinkList->tail);
	DEBUG("tail = %p\n", pstLinkList->tail);
	DEBUG("traverse = %p\n", pstLinkList->traverse);
	DEBUG("print = %p\n", pstLinkList->print);
*/	
	return pstLinkList;
}

/*析构函数*/
int destroy_link_list(link_list_class_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	
	if(_this->size(_this))
	{
		/*清空链表*/
		_this->clear(_this);
	}

	/*释放头结点*/
	free(_this->pHead);
	_this->pHead = _this->pTail = NULL;
	
	free(_this);
	_this = NULL;
	return 0;
}



