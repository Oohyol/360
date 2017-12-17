#define _CRT_SECURE_NO_WARNINGS

#include "web_clause.h"
#include <string>
#include <stdio.h>
#include <string.h>
using std::string;
using std::wstring;
sentence_link::sentence_link( unsigned long para_idx, unsigned long sent_idx, char* msg_buff , int is_begin) {

    this->para_idx = para_idx;
    this->sent_idx = sent_idx;
	this->sent_init = is_begin;
    if( NULL != msg_buff ) {
        this->msg_buff = new char[strlen( msg_buff ) + 1];
        strcpy( this->msg_buff, msg_buff );
    }
    else {
        this->msg_buff = NULL;
    }
    this->next = NULL;

}

sentence_link::~sentence_link() {

    if( NULL != this->msg_buff ) {
        delete[] this->msg_buff;
    }
    this->next = NULL;
}

mapreduce_c::mapreduce_c( const char* config_file ) {
	this->m_load_dict(config_file);
}

mapreduce_c::~mapreduce_c() {
	this->m_unload_dict();
}
int str2int(wchar_t* str)
{
	unsigned long ret = 0;
	int i = 0;
	while( str[i] != L'\0' )
	{
		ret *= 10;
		ret += str[i] - L'0';
		i++;
	}
	return ret;
}
bool mapreduce_c::m_load_dict(const char* file) {

    FILE* fp = NULL;
    if( NULL == ( fp = fopen( file ,"r" ) ) ) {
        return false;
    }
    char buf[200];
    char in_dict_flag[2]="1";
    if( fgets( buf, 200, fp ) ) {
        string_c::m_trim( buf );
        if(! strcmp(buf , "#no-breaking")  )
		{
			this->m_nobreak_dict = new hashtab_c();
			while( fgets( buf, 200, fp ) , buf[0] != '#' ){
				string_c::m_trim( buf );
				wchar_t* val = string_c::m_UTF8ToUnicode( buf );
				//wchar_t* val = EncodeConversion::UTF8ToUnicode( buf );
				this->m_nobreak_dict->m_update( val, wcslen( val ) + 1, in_dict_flag, strlen( in_dict_flag ) + 1 );
				delete[] val;
			}

			string_c::m_trim( buf );
		}
		if( ! strcmp(buf , "#chinese punctuation") )
		{
			while( fgets(buf , 200 , fp) , buf[0] != '#' ){
				string_c::m_trim( buf );
				wchar_t* val = string_c::m_UTF8ToUnicode( buf );
				//wchar_t* val = EncodeConversion::UTF8ToUnicode( buf );
				int ind = 0;
				for( ; ind < (int)wcslen( val ) && val[ind] != L'\t'; ind++);
				val[ ind ] = L'\0';
				if( ! wcscmp( val , L"ZH_LENGTH") )
					//this->m_cn_max_wdnum_per_sent = _wtol(val+ind+1);
					this->m_cn_max_wdnum_per_sent = str2int(val+ind+1);
				else if(! wcscmp( val , L"is_left_seg")){
					if(!wcscmp(val+ind+1,L"true")){
						m_is_left_seg = true;
					}else{
						m_is_left_seg = false;
					}
				}
				else if( ! wcscmp( val , L"first_seg") ){
					for(int i = ind + 1; val[i] != L'\0'; i++)
						m_cn_first_seg.insert( val[i] );
				}
				else if( ! wcscmp( val , L"second_seg") ){
					for(int i = ind + 1; val[i] != L'\0'; i++)
						m_cn_second_seg.insert( val[i] );
				}
				else if( ! wcscmp( val , L"other_seg") ){
					for(int i = ind + 1; val[i] != L'\0'; i++)
						m_cn_other_seg.insert( val[i] );
				}
				else{
					for(int i = ind + 1; val[i] != L'\0'; i++){
						m_cn_syml_seg[ val[i] ] = 0 ;
						if( val[i] == L'{' ) 
							val[i] ++;
						m_cn_symr_seg[ val[i]+1 ] = 0 ;
					}
				}
				delete[] val;
			}
			string_c::m_trim( buf );
		}
		if( ! strcmp( buf , "#english punctuation") )
		{
			while( fgets( buf , 200 , fp) != NULL ){
				string_c::m_trim( buf );
				wchar_t* val = string_c::m_UTF8ToUnicode( buf );
				//wchar_t* val = EncodeConversion::UTF8ToUnicode( buf );
				int ind = 0;
				for( ; ind < (int)wcslen( val ) && val[ind] != L'\t'; ind++);
				val[ ind ] = L'\0';
				if( ! wcscmp( val , L"EN_LENGTH") )
					//this->m_en_max_wdnum_per_sent = _wtol(val+ind+1);
					this->m_en_max_wdnum_per_sent = str2int(val+ind+1);
				else if( ! wcscmp( val , L"first_seg") ){
					for(int i = ind + 1; val[i] != L'\0'; i++)
						m_en_first_seg.insert( val[i] );
				}
				else if( ! wcscmp( val , L"symmetry_l_seg") ){
					for(int i = ind + 1; val[i] != L'\0'; i++) {
						m_en_symmetry_l_seg[ val[i] ] = 0;
						if( val[i] == L'\(') val[i]--;
						m_en_symmetry_r_seg[ val[i]+2 ] = 0;
					}
				}
				else if( ! wcscmp( val , L"second_seg") ){
					for(int i = ind + 1; val[i] != L'\0'; i++)
						m_en_second_seg.insert( val[i] );
				}
				else if( ! wcscmp( val , L"other_seg") ){
					for(int i = ind + 1; val[i] != L'\0'; i++)
						m_en_other_seg.insert( val[i] );
				}
				delete[] val;
			}
		}
	}
    fclose( fp );
    return true;
}

bool mapreduce_c::m_unload_dict() {

    delete this->m_nobreak_dict;
    return true;

}

sentence_link* mapreduce_c::m_map_task_to_sub_jobs_ch( const char* task_text, unsigned long* para_num, unsigned long** sent_num_arr ) {

    unsigned long task_text_len = strlen( task_text );
    unsigned long task_text_temp_len = 0;
    char* task_text_temp = new char[task_text_len + 2];
	
    memset( task_text_temp, 0, sizeof( char ) * ( task_text_len + 2 ) );
	//fprintf(stderr,"%s\n",task_text);
	bool flag = false;
    for( unsigned long i = 0; i < task_text_len; i++ ) 
	{
        if( '\r' != task_text[i] ) 
		{
			if(task_text[i]== ' '){
				if(flag == false){
					task_text_temp[task_text_temp_len++] = task_text[i];
				}
				flag =true;
			}else{
				task_text_temp[task_text_temp_len++] = task_text[i];
				flag = false;
			} 
        }
    }

	if(task_text_temp_len>0&&task_text_temp[task_text_temp_len - 1] ==' '){
		task_text_temp[task_text_temp_len - 1] ='\n';
	}
	if( task_text_temp_len>2&&'\n' == task_text_temp[task_text_temp_len - 1]&&task_text_temp[task_text_temp_len - 2]== ' ' ){
		task_text_temp[task_text_temp_len - 2] ='\n';
		task_text_temp[task_text_temp_len - 1] ='\0';
		task_text_temp_len --;
	}
    if( task_text_temp_len>0&&'\n' != task_text_temp[task_text_temp_len - 1] ) {
        
		strcat( task_text_temp, "\n" );

    }
	//fprintf(stderr,"%s\n",task_text_temp);
	//fprintf(stderr,"out the string");
    wchar_t* content_unicode = string_c::m_UTF8ToUnicode( task_text_temp );
	//wchar_t* content_unicode = EncodeConversion::UTF8ToUnicode( task_text_temp );
    delete[] task_text_temp;
    
    unsigned long para_num_loc = 0;
    unsigned long *sent_num_arr_loc=new unsigned long[1000];
    memset(sent_num_arr_loc,0,sizeof(unsigned long)*1000);
    unsigned long Multi=1;
    sentence_link* head = NULL;
    sentence_link* end = NULL;
    unsigned long text_len = wcslen( content_unicode );
    unsigned long TempCount=0;//记录截取子句的开始位置
    unsigned long CountChWords=0;//统计中文字个数
	unsigned long first_index = 0;//第一个一级点位置
	unsigned long second_index = 0;//第一个对称的二级点位置
	unsigned long tmp_second_index = 0;//第一个二级点位置
	unsigned long other_index = 0;//第一个其他点位置
	map<wchar_t , int> mpl(m_cn_syml_seg);//对称左标号
	map<wchar_t , int> mpr(m_cn_symr_seg);//对称右标号
	int sentence_begin_flag =1;
	for(unsigned long i = 0;i < text_len; i++) 
	{
		if (content_unicode[i] > 0x4e00 && content_unicode[i] < 0x9fa5) {
			CountChWords++;
		}
		else 
		{
			if( mpl.find(content_unicode[i]) != mpl.end() )
				mpl[ content_unicode[i] ]++;//统计左标号个数
			else if( mpr.find(content_unicode[i]) != mpr.end() )
				mpr[ content_unicode[i] ]++;//统计右标号个数
			
			//else if(other_index == 0 &&  m_cn_other_seg.find( content_unicode[i] ) != m_cn_other_seg.end() &&  i-TempCount > 3)
			//	other_index = i;//统计第一个其他标点位置
			else if(m_cn_other_seg.find( content_unicode[i] ) != m_cn_other_seg.end() &&  i-TempCount > 3){
				if(other_index ==0)
					other_index = i;
				if(m_is_left_seg == false){// 从最后一个切割
					other_index = i;
				}
				
			}
			//else if( second_index == 0 && m_cn_second_seg.find( content_unicode[i] ) != m_cn_second_seg.end() && i-TempCount > 3)
			else if( m_cn_second_seg.find( content_unicode[i] ) != m_cn_second_seg.end() && i-TempCount > 3)
			{	
				/*
				if( tmp_second_index == 0) tmp_second_index = i;//统计第一个二级标点位置
				map<wchar_t,int>::const_iterator it = mpr.begin();
				for( ; it != mpr.end(); it++ )
				{
				 if(it->first != L'}'){
				  if(it->second != mpl[it->first-1]) break;
				 } 
				 else{
				  if(it->second != mpl[it->first-2]) break;
				 }
				}
				if( it == mpr.end() ) second_index = i;//统计一个二级标点位置
				*/
				if( tmp_second_index == 0) tmp_second_index = i;//统计第一个二级标点位置
				if(m_is_left_seg == false){// 从最后一个切割
					tmp_second_index = i;
				}
				map<wchar_t,int>::const_iterator it = mpr.begin();
				for( ; it != mpr.end(); it++ )
				{
					if(it->first != L'}')
					{
						if(it->second != mpl[it->first-1]) break;
					}
					else
					{
						if(it->second != mpl[it->first-2]) break;
					}
				}
				if( it == mpr.end()) 
				{
					if( second_index ==0)
						second_index = i;//统计一个二级标点位置
					if(m_is_left_seg == false)// 从最后一个切割
						second_index = i;
				}
			}
			else if ( this->m_cn_first_seg.find( content_unicode[i] ) != this->m_cn_first_seg.end() && CountChWords < this->m_cn_max_wdnum_per_sent )//找到一级标点
			{
				if(content_unicode[i] != L'.' || (i > 0 && content_unicode[i-1] > 0x4e00 && content_unicode[i-1] < 0x9fa5 && i < text_len-1 && content_unicode[i+1] > 0x4e00 && content_unicode[i] < 0x9fa5))//排除不是句点的'.'
				{
					while( i < text_len - 1 && mpr.find(content_unicode[i+1]) != mpr.end() ) //判断右边是否有对称标号
					{
						mpr[ content_unicode[i+1] ]++;
						i++; 
					}
					if(first_index == 0) first_index = i;//第一个一级标点位置
					map<wchar_t,int>::const_iterator it = mpr.begin();
					for( ; it != mpr.end(); it++ )//检查从TempCount到i,标号是否对称
					{
						if(it->first != L'}'){
							if(it->second != mpl[it->first-1]) break;
						}
						else{
							if(it->second != mpl[it->first-2]) break;
						}
					}
					if( it == mpr.end() )  //标号对称的话，截取子句，否则放弃，继续遍历
					{
						char *val = this->m_get_sentence(content_unicode,TempCount,i);
						sentence_link* node = new sentence_link( para_num_loc, sent_num_arr_loc[para_num_loc]++,val,sentence_begin_flag);
						sentence_begin_flag =1;
						delete[] val;
						this->m_insert_to_list(node,head,end);
						first_index = 0;
						CountChWords=0;
						second_index = 0;
						other_index = 0;
						tmp_second_index = 0;
						for(map<wchar_t,int>::const_iterator it = mpr.begin(); it != mpr.end(); it++)
						{
							mpr[it->first] = 0;
							if(it->first != L'}')
								mpl[it->first-1] = 0;
							else
								mpl[it->first-2] = 0;
						}
					}
				}
			}
			else if (content_unicode[i]== L'\n') //遇到换行符
			{
				if (TempCount < i) 
				{
					char *val = this->m_get_sentence(content_unicode,TempCount,i-1);
					sentence_link* node = new sentence_link( para_num_loc, sent_num_arr_loc[para_num_loc]++,val,sentence_begin_flag);
					sentence_begin_flag =1;
					delete[] val;
					this->m_insert_to_list(node,head,end);
					++para_num_loc;
					if ((1000*Multi)==(para_num_loc+1)) {
						sent_num_arr_loc = this->m_realloc_c(sent_num_arr_loc,1000*(++Multi),para_num_loc);
					}
					sent_num_arr_loc[para_num_loc]=0;
					TempCount++;
					CountChWords=0;
					first_index = 0ƒ
								  second_index = 0;
					other_index = 0;
					tmp_second_index = 0;
					for(map<wchar_t,int>::const_iterator it = mpr.begin(); it != mpr.end(); it++)
					{
						mpr[it->first] = 0;
						if(it->first != L'}')
							mpl[it->first-1] = 0;
						else
							mpl[it->first-2] = 0;
					}
				}
				else
				{
					if (sent_num_arr_loc[para_num_loc]==0) {//遇到连续换行符
						sentence_link* node = new sentence_link( para_num_loc, sent_num_arr_loc[para_num_loc],NULL,sentence_begin_flag);   //remove ++ by duquan
						this->m_insert_to_list(node,head,end);
						sentence_begin_flag =1;
					}
					++para_num_loc;//段数++
					if ((1000*Multi)==(para_num_loc+1)) {//段数不够，增加段数
						sent_num_arr_loc = this->m_realloc_c(sent_num_arr_loc,1000*(++Multi),para_num_loc);
					}
					sent_num_arr_loc[para_num_loc]=0;
					TempCount++;
				}
			}
		}
		if(CountChWords >= this->m_cn_max_wdnum_per_sent) //到达长度限制
		{
			// 有一级切割点，但是没有对称，没切
			unsigned long ind = first_index != 0 ? first_index : second_index;
			if( ind == 0 ){
				if( tmp_second_index != 0 )
					ind = tmp_second_index ;
				else
					ind = other_index == 0 ? i : other_index ;//按照优先级，获取能截图的子句位置
			}
			char *val = this->m_get_sentence(content_unicode,TempCount,ind);//截取子句
			sentence_link* node = new sentence_link( para_num_loc, sent_num_arr_loc[para_num_loc]++,val,sentence_begin_flag);//新建节点
			delete[] val;
			this->m_insert_to_list(node,head,end);//插入子句
			sentence_begin_flag = 0;
			CountChWords=0;//清零
			first_index = 0;
			second_index = 0;
			other_index = 0;
			tmp_second_index = 0;
			for(map<wchar_t,int>::const_iterator it = mpr.begin(); it != mpr.end(); it++){
				mpr[it->first] = 0;
				if(it->first != L'}')
					mpl[it->first-1] = 0;
				else
					mpl[it->first-2] = 0;
			}
			i = ind;//回退,想法改进
		}	
	}
	if (TempCount<text_len) {
		char *val = this->m_get_sentence(content_unicode,TempCount,text_len-1);
		sentence_link* node = new sentence_link( para_num_loc, sent_num_arr_loc[para_num_loc]++,val,sentence_begin_flag);
		delete[] val;
		this->m_insert_to_list(node,head,end);
	}
	delete[] content_unicode;
	*para_num = para_num_loc;
	*sent_num_arr = new unsigned long[para_num_loc];
	memcpy( *sent_num_arr, sent_num_arr_loc, sizeof( unsigned long ) * para_num_loc );
	delete[] sent_num_arr_loc;
	return head;

}

char* mapreduce_c::m_get_sentence( wchar_t* buff, unsigned long& TempCount, unsigned long i ) {

	unsigned long Length=i-TempCount+1;
	wchar_t *temp=new wchar_t[Length+1];
	memset(temp,L'\0',sizeof(wchar_t)*(Length+1));
	for (unsigned long count=0;count < Length ;count++) //可以在上面拷贝?
	{
		temp[count]=buff[TempCount+count];
	}
	TempCount=i+1;
	char *val = string_c::m_UnicodeToUTF8( temp );
	//char *val = EncodeConversion::UnicodeToUTF8( temp );
	delete[] temp;
	return val;

}


void mapreduce_c::m_insert_to_list( sentence_link* node, sentence_link* & head, sentence_link* & end ) {

    if (head==NULL) 
	{
        node->next = head;
        head = node;
        end=node;
    }
    else 
	{
        node->next=NULL;
        end->next=node;
        end=node;
    }

}

unsigned long* mapreduce_c::m_realloc_c( unsigned long* sent_num_arr_loc, unsigned long NewSize, unsigned long CurrentSize ) {

    unsigned long *Temp=new unsigned long[NewSize];
    memset(Temp,0,sizeof(unsigned long)*NewSize);
    memcpy( Temp, sent_num_arr_loc, sizeof( unsigned long ) * CurrentSize );
    delete[] sent_num_arr_loc;
    sent_num_arr_loc=Temp;
    Temp=NULL;
    return sent_num_arr_loc;

}

bool mapreduce_c::check_no_breaking(wchar_t* buff , int i ){
	
	wchar_t tmp = buff[i+1];
	buff[i+1] = L'\0';
	wchar_t* p = buff + i;
	int j = i;
	while(j >= 0 && buff[j] != L' '){
		j--;
	}
	int len = i-j+1;
	p = buff +j+1;
	void* val = this->m_nobreak_dict->m_search( p ,len);// hash 中存储的长度都自动加1，所以此处需要同样处理
	
	if(val != NULL){
		buff[i+1] = tmp;
		return false;
	}
#if 0
	for( int j = i; j > i - 7 && i > 6 ; j-- )//hard code
	{
		if( buff[j] == L' '){// add by yangmurun 20150428 
			p = buff +j+1;
			//		printf("the kong postion %d\n",j);
			void* val = this->m_nobreak_dict->m_search( p ,i-j+1);
			//void* val = this->m_nobreak_dict->m_search( p-- ,i-j+2);
			if(val != NULL){
				buff[i+1] = tmp;
				return false;
			}
			//	std::cout<<"the kong postion "<<j<<endl;
			break;
		}
	}
#endif
	buff[i+1] = tmp;
	return true;
}

sentence_link* mapreduce_c::m_map_task_to_sub_jobs_en( const char* task_text , unsigned long* para_num , unsigned long** sent_num_arr ) {//跟空格有关，多空格需要处理,暂时都转成unicode，处理其他语言也方便

	unsigned long task_text_len = strlen( task_text );
	unsigned long task_text_temp_len = 0;
	char* task_text_temp = new char[task_text_len + 2];
	memset( task_text_temp , 0 , sizeof( char ) * ( task_text_len + 2 ) );
	bool flag = false;
    for( unsigned long i = 0; i < task_text_len; i++ ) {
        if( '\r' != task_text[i] ) {
			if(task_text[i]== ' '){
				if(flag == false){
					task_text_temp[task_text_temp_len++] = task_text[i];
				}
				flag =true;
			}else{
				task_text_temp[task_text_temp_len++] = task_text[i];
				flag = false;
			} 
        }
    }

	if(task_text_temp_len>0&&task_text_temp[task_text_temp_len - 1] ==' '){
		task_text_temp[task_text_temp_len - 1] ='\n';
	}
	if( task_text_temp_len>2&&'\n' == task_text_temp[task_text_temp_len - 1]&&task_text_temp[task_text_temp_len - 2]== ' ' ){
		task_text_temp[task_text_temp_len - 2] ='\n';
		task_text_temp[task_text_temp_len - 1] ='\0';
		task_text_temp_len --;
	}
    if( task_text_temp_len>0&&'\n' != task_text_temp[task_text_temp_len - 1] ) {
        
		strcat( task_text_temp, "\n" );

    }
	wchar_t* content_unicode = string_c::m_UTF8ToUnicode( task_text_temp );
	//wchar_t* content_unicode = EncodeConversion::UTF8ToUnicode( task_text_temp );
	delete[] task_text_temp;
	unsigned long para_num_loc = 0;
	unsigned long *sent_num_arr_loc = new unsigned long[1000];
	memset( sent_num_arr_loc , 0 , sizeof( unsigned long ) * 1000);
	unsigned long Multi = 1;
	sentence_link* head = NULL;
	sentence_link* end = NULL;
	unsigned long text_len = wcslen( content_unicode );
	unsigned long TempCount = 0;
	unsigned long CountEnWords = 0;
	unsigned long first_index = 0;
	unsigned long second_index = 0;
	unsigned long other_index = 0;
	map<wchar_t , int>mpl(m_en_symmetry_l_seg) , mpr(m_en_symmetry_r_seg);
	set< wchar_t > mp_judge_word_num; ///// alter by sunkunjie 2013/12/23
	mp_judge_word_num.insert(L'-');
	mp_judge_word_num.insert(L')');
	mp_judge_word_num.insert(L'(');
	mp_judge_word_num.insert(L' ');///// alter by sunkunjie 2013/12/23
	unsigned long sq_count = 0;
	unsigned long dq_count = 0;
	unsigned long messy_code_len = 0;
	int sentence_begin_flag =1;
	
	for( unsigned long i = 0; i < text_len; i++)
	{

		/*if(i == 250){
			map<wchar_t, int>::iterator it;
			printf("mpl\n");
			for(it = mpl.begin();it!= mpl.end();it++){
			 printf("%d ", it->second);
			}
			printf("\nmpr\n");
			for(it = mpr.begin();it!= mpr.end();it++){
			 printf("%d  ", it->second);
			}
			printf("\n");

			printf("%d %d\n",first_index,second_index);
		   }
		   */

		messy_code_len++;
		if( content_unicode[i] > L'Z' && content_unicode[i] < L'a' || content_unicode[i] < L'A' || content_unicode[i] > L'z' )
		{
			if( i > 0 && mp_judge_word_num.find( content_unicode[i-1] ) == mp_judge_word_num.end() && mp_judge_word_num.find( content_unicode[i] ) != mp_judge_word_num.end()  ) 
				CountEnWords++;
			//if(content_unicode[i] == L'\'')//统计单引号个数
			//	sq_count++;
			//else
			if(content_unicode[i] == L'"')//统计双引号个数
				dq_count++;         
			else if(mpl.find( content_unicode[i] ) != mpl.end() )//统计'(''[''{'个数
				mpl[ content_unicode[i] ]++;
			else if(mpr.find( content_unicode[i] ) != mpr.end() )//统计')'']''}'个数
			{
				mpr[ content_unicode[i] ]++;
				//printf("mpr: %d\n",i);
			}
			else if( content_unicode[i] == L' ' )
			{
				if(i == 0 || content_unicode[i-1] == L' ')continue;//防止连续的空格
				//CountEnWords++;//按空格统计单词
				unsigned long tmp_index = i-1;
				if( CountEnWords < this->m_en_max_wdnum_per_sent ) 
				{
					while(tmp_index > 0 && ( content_unicode[tmp_index] == L'"' || content_unicode[tmp_index] == L'\'' \
											 ||  mpr.find(content_unicode[tmp_index]) != mpr.end() ) )
					{
						//if( content_unicode[tmp_index] == L'"' ) dq_count++;//统计单引号个数
						//else if( content_unicode[tmp_index] == L'\'' ) sq_count++;//统计双引号个数
						//else {
						//	mpr[ content_unicode[tmp_index] ]++;//统计')'']''}'个数
						//	printf("mpr: %d\n",tmp_index);
						//}
						tmp_index--;
					}

					

					if( m_en_first_seg.find( content_unicode[tmp_index] ) != m_en_first_seg.end() && ( content_unicode[tmp_index] != L'.' \
							|| ( tmp_index - TempCount > 2 && this->check_no_breaking(content_unicode , tmp_index))) || (CountEnWords < 5 && messy_code_len >= 100) ) 
					{                                             //遇到1 ".?!;"           2 过滤2. a. 以及no-breaking符号 3 乱码输入
						if(first_index == 0) first_index = i;//遇到第一个一级标点的位置
						map<wchar_t , int>::const_iterator it = mpr.begin();
						for( ; it != mpr.end() ; it++ )//遇到一级标点前，检查对称标点是否对称
						{
							int tmp = it->first - 1;
							if(it->first != L')')tmp--;//对称标点不一定相邻
							if(it->second != mpl[ tmp ]) break;
						}
						if( it == mpr.end() && sq_count % 2 == 0 && dq_count % 2 == 0 ) //如果满足条件
						{

							


							char* val = this->m_get_sentence(content_unicode , TempCount , i);//截取满足条件的子句
							sentence_link* node = new sentence_link( para_num_loc , sent_num_arr_loc[para_num_loc]++ , val,sentence_begin_flag );//新建节点
							sentence_begin_flag =1;
							if(CountEnWords < 5 && messy_code_len >= 100)
								sentence_begin_flag =0;
							delete[] val;
							this->m_insert_to_list( node , head , end );//插入到链表里
							first_index = 0;//第一个一级点位置清零
							second_index = 0;//第一个二级点位置清零
							other_index = 0;//第一个其他标点
							for(map<wchar_t , int>::const_iterator it = mpr.begin(); it != mpr.end(); it++)//对称标点个数清零
							{
								mpr[it->first] = 0;
								if(it->first != L')')
									mpl[it->first - 2] = 0;
								else
									mpl[it->first - 1] = 0;
							}
							sq_count = 0;//单引号个数清零
							dq_count = 0;//双引号个数清零
							CountEnWords = 0;//英文词数清零
							messy_code_len = 0;
						}
					}
					else if( second_index == 0 && m_en_second_seg.find( content_unicode[tmp_index] ) != m_en_second_seg.end() ){
						second_index = i;//第一个二级标点位置，暂不考虑对称问题，等有时间在考虑
					}
					else if( other_index == 0 && m_en_other_seg.find( content_unicode[tmp_index] ) != m_en_other_seg.end() ){
						other_index = i;//第一个其他标点位置
					}
				}
				else 
				{
					unsigned long ind = first_index != 0 ? first_index : second_index ;
					if( ind == 0 )
						ind = other_index != 0 ? other_index : i;//获取能截取的字串位置
					char* val = this->m_get_sentence(content_unicode , TempCount , ind);//截取满足条件的子句
					sentence_link* node = new sentence_link( para_num_loc , sent_num_arr_loc[para_num_loc]++,val,sentence_begin_flag);//新建节点
					sentence_begin_flag = 0;
					delete[] val;
					this->m_insert_to_list( node , head , end );//插入到链表里
					first_index = 0;//清零
					second_index = 0;
					other_index = 0;
					for(map<wchar_t , int>::const_iterator it = mpr.begin(); it != mpr.end(); it++)
					{
						mpr[it->first] = 0;
						if(it->first != L')')
							mpl[it->first-2] = 0;
						else
							mpl[it->first-1] = 0;
					}
					sq_count = 0;
					dq_count = 0;
					CountEnWords = 0;
					i = ind;
					messy_code_len = 0;
				}

				/*if (messy_code_len == 400){
				 printf("%d\n ",messy_code_len);
				}*/
			}
			else if( content_unicode[i] == L'\n' ) //遇到换行符
			{
				if( TempCount < i )//如果遇到换行符之前没有截取子句
				{
					char* val = this->m_get_sentence( content_unicode , TempCount , i-1);
					sentence_link* node = new sentence_link( para_num_loc , sent_num_arr_loc[para_num_loc]++ , val ,sentence_begin_flag );
					sentence_begin_flag =1;
					delete[] val;
					this->m_insert_to_list(node , head , end );
					para_num_loc++;
					if (1000 * Multi == (para_num_loc+1) )
					{
						sent_num_arr_loc = this->m_realloc_c( sent_num_arr_loc , 1000 * (++Multi) , para_num_loc);
					}
					sent_num_arr_loc[para_num_loc] = 0;
					TempCount++;
					first_index = 0;
					second_index = 0;
					other_index = 0;
					for(map<wchar_t , int>::const_iterator it = mpr.begin(); it != mpr.end(); it++)
					{
						mpr[it->first] = 0;
						if(it->first != L')')
							mpl[it->first-2] = 0;
						else
							mpl[it->first-1] = 0;
					}
					sq_count = 0;
					dq_count = 0;
					CountEnWords = 0;
					messy_code_len = 0;
				}
				else
				{
					if(sent_num_arr_loc[para_num_loc] == 0){//遇到连续换行处理
						sentence_link* node = new sentence_link( para_num_loc , sent_num_arr_loc[para_num_loc] , NULL ,sentence_begin_flag);   //remove ++ by duquan
						this->m_insert_to_list( node , head , end );
						sentence_begin_flag =1;
					}
					para_num_loc++;//下一段开始
					if (1000 * Multi == (para_num_loc+1) )
					{
						sent_num_arr_loc = this->m_realloc_c( sent_num_arr_loc , 1000 * (++Multi) , para_num_loc);
					}
					sent_num_arr_loc[para_num_loc] = 0;
					TempCount++;	
				}
			}
		}
		if(CountEnWords < 1 && messy_code_len >= 100 || messy_code_len >= 400)// CountEnWords 应该小于一个阈值  或者长度大于200直接cut
		{

			char* val = this->m_get_sentence(content_unicode , TempCount , i);//截取满足条件的子句
			sentence_link* node = new sentence_link( para_num_loc , sent_num_arr_loc[para_num_loc]++,val ,sentence_begin_flag );//新建节点
			sentence_begin_flag = 0;
			delete[] val;
			this->m_insert_to_list( node , head , end );//插入到链表里
			first_index = 0;//清零
			second_index = 0;
			other_index = 0;
			for(map<wchar_t , int>::const_iterator it = mpr.begin(); it != mpr.end(); it++)
			{
				mpr[it->first] = 0;
				if(it->first != L')')
					mpl[it->first-2] = 0;
				else
					mpl[it->first-1] = 0;
			}
			sq_count = 0;
			dq_count = 0;
			CountEnWords = 0;
			messy_code_len = 0;
		}
	}
	if( TempCount < text_len )//获取末尾子句
	{
		char* val = this->m_get_sentence( content_unicode , TempCount , text_len-1 );
		sentence_link* node = new sentence_link( para_num_loc , sent_num_arr_loc[para_num_loc]++ , val ,sentence_begin_flag);
		sentence_begin_flag =1;
		delete[] val;
		this->m_insert_to_list( node , head , end );
	}
	delete[] content_unicode;
	*para_num = para_num_loc;//返回段落数
	*sent_num_arr = new unsigned long[para_num_loc];
	memcpy( *sent_num_arr , sent_num_arr_loc , sizeof( unsigned long ) * para_num_loc );//返回每段多少句
	delete[] sent_num_arr_loc;
	return head;
}

