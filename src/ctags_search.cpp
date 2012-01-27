// ctags_search.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//
#include <ctype.h>
#include <assert.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

class AutoFree{
public:
	AutoFree(void*p) : m_ptr(p){
	}
	~AutoFree(){
		if(m_ptr){
			free(m_ptr);
			m_ptr=0;
		}
	}
protected:
private:
	AutoFree(const AutoFree&);
	AutoFree &operator=(const AutoFree&);
	void	*m_ptr;
};

class AutoClose{
public:
	AutoClose(FILE* fp) : m_fp(fp){
	}
	~AutoClose(){
		if(m_fp){
			fclose(m_fp);
			m_fp=0;
		}
	}
protected:
private:
	AutoClose(const AutoClose&);
	AutoClose &operator=(const AutoClose&);
	FILE*	m_fp;
};


static char * strcasestr (char *haystack, char *needle){
	char *p, *startn = 0, *np = 0;

	for (p = haystack; *p; p++) {
		if (np) {
			if (tolower(*p) == tolower(*np)) {
				if (!*++np)
					return startn;
			} else
				np = 0;
		} else if (tolower(*p) == tolower(*needle)) {
			np = needle + 1;
			startn = p;
		}
	}

	return 0;
}


static void usage(){
	printf(
	"Usage: ctags_search PATTERN FILE\n"
	"Search PATTERN in the FILE.\n"
	"Example: ctags_search MyClassName c:\\project\\tags\n"
	);
	/*printf(
	"Usage: ctags_search MAX PATTERN FILE\n"
	"Search by up to MAX PATTERN in the FILE.\n"
	"Example: ctags_search MyClassName c:\\project\\tags\n"
	);*/
}

static char* read_file(FILE*fp){
	const size_t filesize = _filelength(_fileno(fp));
	
	//+3 == '\n' '\n' '\0'
	char*dst_top = (char*)malloc(filesize+2);
	if(! dst_top){
		printf("error: not enought memory.\n");
		return 0;
	}
	char*dst=dst_top;
	
	*dst='\n';
	++dst;
	
	size_t read_size = fread(dst, 1, filesize, fp);
	if(! read_size){
		free(dst_top);
		printf("error: file read error.\n");
		return 0;
	}
	
	dst += read_size;
	*dst = '\n';
	++dst;
	
	*dst = '\0';
	++dst;
	
	return dst_top;
}

static char* make_pattern(const char*pattern, size_t pattern_len){
	//+3 == '\n' '\t' '\0'
	char *new_pattern = (char*)malloc(3+pattern_len);
	if(!new_pattern){
		printf("error: not enought memory.\n");
		return 0;
	}
	
	char* dst = new_pattern;
	
	*dst = '\n';
	++dst;
	
	strcpy(dst, pattern);
	dst+=pattern_len;
	*dst = '\t';
	++dst;

	*dst='\0';

	return new_pattern;
}

static bool find_all(char*pattern, char*file_image){
	while(1){
		const char*find_str = /*strstr*/ strcasestr(file_image, pattern);
		if(pattern == find_str){
			//長さ0 ？
			printf("error: strlen is 0 ?.\n");
			return false;
		}
		if(find_str==0){
			//printf("not found pattern.\n");
			return false;
		}
		assert((*find_str) == '\n');
		++find_str;
	
		//改行までを表示する
		char*find_ret = (char*)strchr(find_str, '\n');
		if(!find_ret){
			assert("internal error." && 0);
			return false;
		}
		
		{
			const char old = *find_ret;
			*find_ret='\0';
			printf("%s\n",find_str);
			*find_ret=old;
		}
		
		file_image = find_ret;
	}
	
	return true;
}

int main(int argc, char* argv[]){
	if(3 != argc){
		usage();
		return 1;
	}
	
	const char* in_pattern = argv[1];
	const char* in_filename= argv[2];
	
	const size_t pattern_len = strlen(in_pattern);
	if(! pattern_len){
		printf("error: pattern is empty.\n");
		return 1;
	}
	
	{
		FILE*fp = fopen(in_filename, "rtS");
		if(! fp){
			printf("error: File open error.\n");
			return 1;
		}
		AutoClose	auto_close_fp(fp);
		
		char*file_image = read_file(fp);
		if(! file_image){
			return 1;
		}
		AutoFree	auto_free_file_image(file_image);
		
		{
			char* new_pattern = make_pattern(in_pattern, pattern_len);
			if(!new_pattern){
				return 0;
			}
			AutoFree	auto_free_new_pattern(new_pattern);
			
			if(! find_all(new_pattern,file_image)){
				return 0;
			}
		}
	}
	
	return 0;
}

