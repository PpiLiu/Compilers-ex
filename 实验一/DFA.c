#include<stdio.h>
#include<stdlib.h>
#include<string.h>

typedef struct{
    int cur,next;
    char chr;
}TRANS_FUN;

typedef struct{
    int zifu_n;
    char *zifu;    //保存字符集
    int state_n,goal_n;
    int *state;    //保存是否为目标状态
    int trfun_n;   //转移函数个数
    TRANS_FUN *trfun;
    int **table;
}DFA;


DFA dfa1;
int ha[256];      //用于查询字符是否在字符集中
int *stack;       //留作dfs使用
char *tstr;        //同上，留作dfs时保存字符串


int DFA_INPUT();    //手动输入DFA
int DFA_FINPUT();   //文件读取DFA
int DFA_FOUTPUT();  //DFA文件生成
int DFA_check(DFA *df);
int DFA_free(DFA *df); //释放dfa中动态分配的内存
int DFA_createtable(DFA *df); //为DFA创建转移表

int str_check(DFA *df,char *str1);//检查str1是否满足该dfa
int str_make(DFA *df,int n);      //生成长度小于等于n的所有满足dfa的字符串
void dfs(DFA *df,int n/*最大深度*/,int cur/*当前深度*/);

int main()
{
    int flag;
    memset(ha,0,sizeof(ha)); //初始化字符集查询表
    printf("请选择DFA的读取方式:\n0.手动输入\n1.从DFA文件读取\n请输入数字:");
    scanf("%d",&flag);
    if(flag)
    {//文件读取
        DFA_FINPUT();
        DFA_createtable(&dfa1);
    }else
    {//手动输入
        DFA_INPUT();
        DFA_createtable(&dfa1);
        printf("\n是否需要将该DFA保存为DFA文件？ y/n\n");
        scanf("%c",&flag);
        fflush(stdin);
        if(flag == 'y')
            DFA_FOUTPUT();
        else if(flag == 'n')
            ;
        else
            printf("输入格式错误,默认不保存文件;");
    }

    //功能菜单
    flag = 1;
    int tmpi;
    char tmpstr[90];
    while(flag)
    {
        printf("\ndfa已成功读取，请输入需要使用的功能\n");
        printf("0.退出程序\n");
        printf("1.dfa错误检查\n");
        printf("2.生成长度小于等于n的所有规则字符串\n");
        printf("3.检验输入字符串是否符合规则\n");
        printf("请输入:");
        fflush(stdin);
        scanf("%d",&flag);
        fflush(stdin);
        switch(flag)
        {
        case 0:
            break;
        case 1:
            DFA_check(&dfa1);
            break;
        case 2:
            printf("\n请输入生成字符串的最大长度:");
            scanf("%d",&tmpi);
            str_make(&dfa1,tmpi);
            break;
        case 3:
            printf("\n请输入需要检验的字符串:\n");
            scanf("%s",tmpstr);
            if(str_check(&dfa1,tmpstr))
                printf("该字符串是规则字符串\n");
            else
                printf("该字符串不是规则字符串");
            break;
        default:
            printf("请输入正确的选项(0~3)\n");
            break;
        }
    }


    DFA_free(&dfa1);
    return 0;
}

int DFA_INPUT()
{
    char ctmp;
    int i,j;
    printf("请按照提示依次输入DFA的五元组\n");

    //字符集
    printf("\n字符集:\n请输入字符集中元素个数:");
    scanf("%d",&dfa1.zifu_n);
    fflush(stdin);
    dfa1.zifu = malloc(sizeof(char) * (dfa1.zifu_n + 1));
    printf("请输入字符集中的字符(不需要分隔):\n");
    for(i=1;i <= dfa1.zifu_n;i++)  //从1到n
    {
        scanf("%c",&dfa1.zifu[i]);
        ha[dfa1.zifu[i]]=i;
    }

    //状态集(初始状态固定为0)
    printf("\n状态集:\n请输入状态集的元素个数n\n:状态名称视为0 ~ (n-1)\n且状态0将视为初始状态:\n");
    scanf("%d",&dfa1.state_n);
    dfa1.state = malloc(sizeof(int) * dfa1.state_n );


    //目标状态
    memset(dfa1.state,0,sizeof(int) * dfa1.state_n ); //目标状态标记初始化为0
    printf("\n目标状态:请输入目标状态的数量:");
    scanf("%d",&dfa1.goal_n);
    printf("请输入目标状态的状态编号\n取值为[0, n-1] 用空格分隔:\n例:2 3 4\n");
    for(i=0;i<dfa1.goal_n;i++)
    {//标记目标状态
        scanf("%d",&j);
        dfa1.state[j] = 1;
    }

    //转移函数
    printf("\n转移函数:\n请输入转移函数的个数:");
    scanf("%d",&dfa1.trfun_n);
    dfa1.trfun = malloc(sizeof(TRANS_FUN) * dfa1.trfun_n);
    printf("请输入转移函数,每行一个,格式为:当前状态(空格)下一状态(空格)接收字符(换行符)\n\
特别注意:每个转移函数只能包括一个字符,若一个状态转移可以接收多种字符,请分成两条转移函数输入,\n\
另,可用##表示\"字符集以外任意字符\"例:\n\
\t0 1 a\n\t1 2 ##\n\t1 3 d\n请输入:\n");
    for(i=0;i<dfa1.trfun_n;i++)
    {
        scanf("%d %d %c",&dfa1.trfun[i].cur,&dfa1.trfun[i].next,&ctmp);
        if(ctmp == '#')//检查是否为"##"
        {
            scanf("%c",&ctmp);
            if(ctmp == '#')
                dfa1.trfun[i].chr = 0; //赋值为0表示other
            else
                dfa1.trfun[i].chr = '#'; //只是普通的'#'字符
        }else
            dfa1.trfun[i].chr = ctmp;
    }
    fflush(stdin);
    printf("----读取完成----\n");
    return 0;
}

int DFA_FINPUT()  //文件读取
{
    FILE *fin;
    char stmp[90];
    char ctmp;
    int i,j;
    printf("\n请输入需要读取的DFA文件名(如:\"aa.dfa\"):");
    scanf("%s",stmp);
    fin = fopen(stmp,"r");   //以读取方式打开

    //字符集
    fscanf(fin,"%d\n",&dfa1.zifu_n);
    dfa1.zifu = malloc(sizeof(char) * (dfa1.zifu_n + 1));
    for(i=1;i <= dfa1.zifu_n;i++)  //从1到n
    {
        fscanf(fin,"%c",&dfa1.zifu[i]);
        ha[dfa1.zifu[i]]=i;
    }

    //状态集(初始状态固定为0)
    fscanf(fin,"%d",&dfa1.state_n);
    dfa1.state = malloc(sizeof(int) * dfa1.state_n );

    //目标状态
    memset(dfa1.state,0,sizeof(int) * dfa1.state_n ); //目标状态标记初始化为0
    fscanf(fin,"%d",&dfa1.goal_n);
    for(i=0;i<dfa1.goal_n;i++)
    {//标记目标状态
        fscanf(fin,"%d",&j);
        dfa1.state[j] = 1;
    }

    //转移函数
    fscanf(fin,"%d",&dfa1.trfun_n);
    dfa1.trfun = malloc(sizeof(TRANS_FUN) * dfa1.trfun_n);
    for(i=0;i<dfa1.trfun_n;i++)
    {
        fscanf(fin,"%d %d %c",&dfa1.trfun[i].cur,&dfa1.trfun[i].next,&ctmp);
        if(ctmp == '#')//检查是否为"##"
        {
            fscanf(fin,"%c",&ctmp);
            if(ctmp == '#')
                dfa1.trfun[i].chr = 0; //赋值为0表示other
            else
                dfa1.trfun[i].chr = '#'; //只是普通的'#'字符
        }else
            dfa1.trfun[i].chr = ctmp;
    }

    fclose(fin);
    printf("----读取完成----\n");
    return 0;
}

int DFA_FOUTPUT()
{
    FILE *fout;
    char stmp[90];
    int i,j;
    printf("请输入需要保存的文件名(如:\"aa.dfa\"):");
    fout = fopen(stmp,"w");

    //字符集
    fprintf(fout,"%d\n",dfa1.zifu_n);
    for(i=1;i <= dfa1.zifu_n;i++)  //从1到n
    {
        fprintf(fout,"%c",dfa1.zifu[i]);
    }

    //状态集(初始状态固定为0)
    fprintf(fout,"\n%d\n",dfa1.state_n);

    //目标状态
    fprintf(fout,"%d\n",dfa1.goal_n);
    j=0; //输出辅助,控制空格
    for(i=0;i<dfa1.state_n;i++)
    {//标记目标状态
        if(j)
            fprintf(fout," ");
        if(dfa1.state[i] == 1)
        {
            fprintf(fout,"%d",i);
            j=1;
        }
    }

    //转移函数
    fprintf(fout,"\n%d\n",dfa1.trfun_n);
    for(i=0;i<dfa1.trfun_n;i++)
    {
        fprintf(fout,"%d %d %c",dfa1.trfun[i].cur,dfa1.trfun[i].next,dfa1.trfun[i].chr);
        if(dfa1.trfun[i].chr == 0)//检查是否为"##"
            fprintf(fout,"##");
        fprintf(fout,"\n");
    }
    fclose(fout);
    printf("----写入完成----\n");
    return 0;
}

int DFA_check(DFA *df)
{
    int i,j;

    //字符集打印
    printf("DFA字符集如下:");
    for(i=1;i<df->zifu_n;i++)
        printf("%c ",df->zifu[i]);
    printf("%c\n",df->zifu[i]);

    //字符集重复检验
    int check[256];
    memset(check,0,sizeof(check));
    for(i=1;i<=df->zifu_n;i++)
    {
        if(check[ df->zifu[i] ])
        {
            printf("存在重复元素\n");
            break;
        }
        check[ df->zifu[i] ] = 1;
    }
    if(i > df->zifu_n)//检验结束未触发break
        printf("不存在重复元素\n");

    //接受状态集检验
    int sum=0;
    i = df->goal_n ;
    printf("\n接受状态有%d个",i);
    if(i<=0)
        printf(",数量异常!");
    else
    {
        printf(",如下:\n");
        for(i=0;i<df->state_n;i++)
        {
            if(df->state[i]>0)
            {
                printf("%d ",i);
                sum++;
            }
        }
        if(sum != df->goal_n)
            printf("\n与数目不相符！\n");
        else
            printf("\n无误\n");
    }

    //转移表打印
    printf("\n转移表如下图:\n   |\t##");
    for(i=1;i<=df->zifu_n;i++)
        printf("\t%c",df->zifu[i]);
    printf("\n");

    printf("----");
    for(i=0;i<=df->zifu_n;i++)
        printf("--------");
    printf("\n");

    for(i=0;i<df->state_n;i++)
    {
        printf("%3d|",i);
        for(j=0;j<=df->zifu_n;j++)
            printf("\t%d",df->table[i][j]);
        printf("\n");
    }

    //转移函数
    int trflag = 0;
    printf("\n转移函数如下(格式为\"当前状态 下一状态 接收字符\"):\n");
    for(i=0;i<df->trfun_n;i++)
    {
        printf("%d %d %c",df->trfun[i].cur,df->trfun[i].next,df->trfun[i].chr);
        if(df->trfun[i].chr == 0)
            printf("##");
        if(df->table[ df->trfun[i].cur ][ ha[ df->trfun[i].chr ] ] != df->trfun[i].next) //检查该函数是否在表中
        {
            printf("  该转移函数未在表中 ！！");
            trflag = 1;
        }
        printf("\n");
    }
    if(!trflag)//状态函数均已在表中，此时可检查表中转移数量是否与函数一致
    {
        for(i=0;i<df->state_n;i++)
            for(j=0;j<=df->zifu_n;j++)
                if(df->table[i][j] != -1) trflag++;
        if(trflag == df->trfun_n)
            printf("表中转移函数数量一致，无错误\n");
        else
            printf("表中转移函数数量不一致，发生错误!\n");
    }else//存在不在表中的状态函数
        printf("表中缺失部分函数!\n");
    return 0;
}

int DFA_free(DFA *df) //释放内存
{
    int i;
    free(df->zifu);
    free(df->state);
    free(df->trfun);
    for(i=0;i<df->state_n;i++)
        free(df->table[i]);
    free(df->table);
    return 0;
}

int DFA_createtable(DFA *df)
{
    int i;

    //分配内存
    df->table = malloc(sizeof(int*) * df->state_n);
    for(i=0;i<df->state_n;i++)
    {//为其他字符多留一个位置
        df->table[i] = malloc(sizeof(int) * (df->zifu_n + 1));
        memset(df->table[i],-1,sizeof(df->table[i]));//将表初始化为-1
    }

    //将转移函数写入表中
    for(i=0;i<df->trfun_n;i++)
        df->table[ df->trfun[i].cur ][ ha[df->trfun[i].chr] ] = df->trfun[i].next;

    return 0;
}

int str_check(DFA *df,char* str1)
{
    int i,len,sta_cur;//当前状态
    len = strlen(str1);
    sta_cur = 0;      //初始状态设为0
    for(i=0;i<len;i++)
    {
        sta_cur = df->table[sta_cur][ ha[ str1[i] ] ];
        if(sta_cur == -1) //进入死状态
            return 0;
    }
    if(df->state[sta_cur])//检查最终状态是否为接受状态
        return 1;
    else
        return 0;
}

int str_make(DFA *df,int n)
{
    //为dfs要使用的栈和临时字符串分配空间
    stack = malloc(sizeof(int) * (n+1) );
    tstr = malloc(sizeof(char) * (n+2) );
    memset(tstr,0,sizeof(char) * (n+2) );
    int i;
    dfs(df,n,0);
    free(stack);
    free(tstr);
    return 0;
}

void dfs(DFA *df,int n/*最大深度*/,int cur/*当前深度*/)
{
    int i,sta;
    if(cur == 0)
        sta = 0;
    else
    {
        sta = stack[cur-1];
        if(df->state[sta])
            printf("%s\n",tstr);
    }

    if(cur == n)
        return ;

    for(i=0;i<=df->zifu_n;i++)
    {
        if(df->table[sta][i] == -1)
            continue;
        tstr[cur] = df->zifu[i];
        stack[cur] = df->table[sta][i];
        dfs(df,n,cur+1);
        tstr[cur] = 0;
    }
}
