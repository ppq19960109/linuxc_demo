#include <stdio.h>
#include <malloc.h>
#define OK 1
#define ERROR 0
typedef int Status;
typedef char TElemType;
typedef struct BiTNode{
TElemType data;
struct BiTNode*lchild,*rchild;
}*BiTree;

void CreateBiTree(BiTree &T) 
{ // 由标明空子树的完整先根遍历序列建立一棵二叉树 
 	char ch;  
 	scanf("%c",&ch);  
 	if(ch=='#') 
	 {
		 T=NULL;   //"#"字符表示空树  
		 printf("CreateBiTree null\n");
	 } 
	else 
	{	T=(BiTree)malloc(sizeof(BiTNode));
		T->data=ch;                 //生成根结点  
		printf("%c\n",T->data);
 	 	CreateBiTree(T->lchild);    //构造左子树   	 
		CreateBiTree(T->rchild);    //构造右子树  
		
 	} 
	 printf("CreateBiTree over\n");
 	//return OK; 
} 

void PreRootTraverse(BiTree T){
	if(T!=NULL)
	{	printf("%c",T->data);
		PreRootTraverse(T->lchild);
		PreRootTraverse(T->rchild);
	}
}
void InRootTraverse(BiTree T){
	if(T!=NULL)
	{	InRootTraverse(T->lchild);
		printf("%c",T->data);
		InRootTraverse(T->rchild);
	}
}
void PostRootTraverse(BiTree T){
	if(T!=NULL)
	{	PostRootTraverse(T->lchild);
		PostRootTraverse(T->rchild);
		printf("%c",T->data);
	}
}
void CopyBiTree(BiTree T, BiTree &TT)
/* 递归复制二叉树T得到TT */
{
    if(T==NULL)
       return;
    else{
       TT=(BiTree)malloc(sizeof(BiTNode));
       TT->data=T->data;
       CopyBiTree(T->lchild,TT->lchild);
       CopyBiTree(T->rchild,TT->rchild);
    }
}
bool check(BiTNode *T1,BiTNode *T2)//判断两棵二叉树是否相等 
{
	if(!T1&&T2)
		return true;
	if(!T1||!T2)
		return false;
	if(T1&&T2){
		if(T1->data!=T2->data)
			return false;
		bool l=check(T1->lchild,T2->lchild);
		bool r=check(T1->rchild,T2->rchild);
		return l&&r;
	}
	return false;
}
void CountLeaf(BiTree T,int &num)
{	if(T!=NULL){
		if((T->lchild==NULL)&&(T->rchild==NULL))
			num++;
		CountLeaf(T->lchild,num);
		CountLeaf(T->rchild,num);
	}
}

int Depth(BiTree T)//求树的深度 
{	int depthLeft=0;
	int depthRight=0;
	int depthval=0;
	if(T!=NULL){
	depthLeft=Depth(T->lchild);
	depthRight=Depth(T->rchild);
	depthval=1+(depthLeft>depthRight?depthLeft:depthRight);
	}
	else
		depthval=0;
	return depthval;
}

int SearchNode(BiTree &T,char x)//查找 
{	if(T!=NULL)
	{	if(T->data==x)
			return OK;
		else if(T->data>x){
			return SearchNode(T->lchild,x);
		}
		else if(T->data<x)
			return SearchNode(T->rchild,x);
	}
	return ERROR;
}
int main(){
	BiTree T;
	BiTree T2;
	int depth=0;
	int count=0;
	char x;
	int a;
	// printf("建立一棵二叉树：(以#表示空)\n");
	// CreateBiTree(T);
	while(1){
		printf("1-建立棵二叉树\n");
		printf("2-复制二叉树\n");
		printf("3-判断两棵二叉树相等\n");
		printf("4-求二叉树的叶子结点的个数\n");
		printf("5-求二叉树的深度\n");
		printf("6-查找\n");
		printf("请选择要进行的操作：\n"); 
		// scanf("%d",&a);
		a=1;
		switch(a){
		
			case 1:
			printf("请先输入完整的先根先序遍历：(以#表示空)\n");
			CreateBiTree(T);
			printf("-----\n");
			printf("先序遍历后：");
			PreRootTraverse(T);
			printf("\n");
			printf("中序遍历后：");
			InRootTraverse(T);
			printf("\n");
			printf("后序遍历后：");
			PostRootTraverse(T);
			printf("\n");
			break;
		
			case 2:

			CopyBiTree(T,T2);
			printf("复制后的后根遍历为：\n");
			PostRootTraverse(T2);
			break;
				
			case 3:
			
			CopyBiTree(T,T2);
			PostRootTraverse(T2);
			if(T2==T)	
				printf("这两棵树是相等的，复制操作成功\n"); 
			else
				printf("这两棵树时不相等的，复制操作不成功\n");
			break;
		
			case 4:
			CountLeaf(T,count);
			printf("树中叶子结点的个数为%d\n",count);
			break;
		
			case 5:
			depth=Depth(T);
			printf("树的深度为%d\n",depth);
			break;
		
			case 6:
			printf("请输入一个指定的字符x：");
			scanf("%c",&x);
			if(SearchNode(T,x))
				printf("OK\n");
			else
				printf("ERROR\n");
			break;
		
			default :
			printf("ERROR\n");	
		}
	}
	return 0;	
}
