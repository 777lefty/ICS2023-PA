#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

static char klib_stdio_buf[1024];

int kint2str(int num, char* str) {
    int i = 0; // 用来记录字符串的长度和位置
    int sign = num; // 用来记录整数的正负号
    if (num < 0) { // 如果是负数，先取绝对值
        num = -num;
    }
    do {
        str[i++] = num % 10 + '0'; // 将整数的最后一位转换为字符，并存储到字符串中
        num /= 10; // 去掉整数的最后一位
    } while (num > 0); // 直到整数为 0 退出循环
    if (sign < 0) { // 如果是负数，添加负号到字符串中
        str[i++] = '-';
    }
    str[i] = '\0'; // 在字符串末尾添加空字符
    // 由于上面的过程得到的字符串是逆序的，所以需要反转字符串的顺序
    int j = 0; // 用来记录反转的起始位置
    int k = i - 1; // 用来记录反转的结束位置
    char temp; // 用来交换两个字符的临时变量
    while (j < k) { // 头尾交换，直到 j >= k
        temp = str[j];
        str[j] = str[k];
        str[k] = temp;
        j++;
        k--;
    }
    return i; // 返回转换后的字符串长度
}

int printf(const char *fmt, ...) {
  //panic("Not implemented");
    va_list ap;
    int len = 0;
    char *p=klib_stdio_buf;
    va_start(ap, fmt);
    while (*fmt){
      int length=0;
      if(*fmt=='%'){
        fmt++;
	int stuff_len=0;
	char stuff='\0';
	if('0'<=*fmt&&*fmt<='9'){
	  stuff=*fmt;
	  fmt++;
	  stuff_len=*fmt-48;
	  fmt++;
	}
	switch (*fmt){
		case 'd':{
		    int d=va_arg(ap, int);
		    length = kint2str(d,p);
		    if(length<stuff_len){
		      while(stuff_len>length){
		        *p=stuff;
			stuff_len--;
			p++;
			len++;
		      }
		      kint2str(d,p);
		    }
		    len+=length;
		    p+=length;
		    break;
		}
		case 's':{
		  char *s=va_arg(ap, char*);
                  int i=0;
		  while(s[i]!='\0'){
		    p[i]=s[i];
		    i++;
		  }
		  length=i;
		  len+=length;
		  p+=length;
		  break;
		}
		case 'c':{
		  char c=(char)va_arg(ap, int);
	          *p++=c;
		  len++;
		  break;		  
		}
	        case 'p':{
		  int ptr=va_arg(ap, int);
                  int length=kint2str(ptr, p);
		  len+=length;
		  p+=length;
		  break;
		}
		default:{
		  *p++='%';
	          *p++=*fmt;
	          len+=2;
		  break;	  
		}	
	}
      }
      else{
        *p++=*fmt;
	len++;
      }
      fmt++;
    }
    *p='\0';
    //len=vsprintf(klib_stdio_buf, fmt, ap);
    va_end(ap);
    putstr(klib_stdio_buf);
    return len;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  //panic("Not implemented");
 int len = 0;
    char *p=out;
    while (*fmt){
      int length=0;
      if(*fmt=='%'){
        fmt++;
	int stuff_len=0;
	char stuff='\0';
	if('0'<=*fmt&&*fmt<='9'){
	  stuff=*fmt;
	  fmt++;
	  stuff_len=*fmt-48;
	  fmt++;
	}
	switch (*fmt){
		case 'd':{
		    int d=va_arg(ap, int);
		    length = kint2str(d,p);
		    if(length<stuff_len){
		      while(stuff_len>length){
		        *p=stuff;
			stuff_len--;
			p++;
			len++;
		      }
		      kint2str(d,p);
		    }
		    len+=length;
		    p+=length;
		    break;
		}
		case 's':{
		  char *s=va_arg(ap, char*);
                  int i=0;
		  while(s[i]!='\0'){
		    p[i]=s[i];
		    i++;
		  }
		  length=i;
		  len+=length;
		  p+=length;
		  break;
		}
		case 'c':{
		  char c=(char)va_arg(ap, int);
	          *p++=c;
		  len++;
		  break;		  
		}
		default:{
		  *p++='%';
	          *p++=*fmt;
	          len+=2;
		  break;	  
		}	
	}
      }
      else{
        *p++=*fmt;
	len++;
      }
      fmt++;
    }
    *p='\0';
    return len;
}

int sprintf(char *out, const char *fmt, ...) {
  //panic("Not implemented");
    va_list ap; // 定义一个 va_list 类型的变量
    int len = 0; // 定义一个输出字符串的长度变量
    char *p = out; // 定义一个指向输出字符串的指针
    va_start(ap, fmt); // 初始化 va_list 变量，使它指向第一个可变参数
    while (*fmt) { // 遍历格式字符串
	int length=0;
        if (*fmt == '%') { // 如果遇到 % 符号，表示后面跟着一个格式码
            fmt++; // 移动到格式码的位置
            switch (*fmt) { // 判断格式码的类型
                case 'd': { // 如果是整数类型
                    int d = va_arg(ap, int); // 从可变参数列表中取出一个 int 类型的参数
                    //len += sprintf(p, "%d", d);
                    length = kint2str(d,p); // 将整数格式化为字符串，并写入输出字符串中，同时更新长度变量
		    len+=length;
                    p += length; // 移动指针到输出字符串的末尾
                    break;
                }
                case 's': { // 如果是字符串类型
                    char *s = va_arg(ap, char *); // 从可变参数列表中取出一个 char * 类型的参数
                    //len += sprintf(p, "%s", s); // 将字符串复制到输出字符串中，同时更新长度变量
                    int i=0;
                    while(s[i]!='\0'){
                        p[i]=s[i];
                        i++;
                    }
                    length=i;
                    len+=length;
                    p += length; // 移动指针到输出字符串的末尾
                    break;
                }
                case 'c': { // 如果是字符类型
                    char c = (char) va_arg(ap, int); // 从可变参数列表中取出一个 int 类型的参数，并强制转换为 char 类型
                    *p++ = c; // 将字符写入输出字符串中
                    len++; // 更新长度变量
                    break;
                }
                // 其他格式码可以类似地处理
                default: { // 如果是无效的格式码
                    *p++ = '%'; // 将 % 符号写入输出字符串中
                    *p++ = *fmt; // 将格式码写入输出字符串中
                    len += 2; // 更新长度变量
                    break;
                }
            }
        } else { // 如果不是 % 符号，表示是普通的字符
            *p++ = *fmt; // 将字符写入输出字符串中
            len++; // 更新长度变量
        }
        fmt++; // 移动到下一个字符的位置
    }
    *p = '\0'; // 在输出字符串的末尾添加空字符
    va_end(ap); // 结束可变参数的获取
    return len; // 返回输出字符串的长度
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  //panic("Not implemented");
    va_list ap; // 定义一个 va_list 类型的变量
    int len = 0; // 定义一个输出字符串的长度变量
    char *p = out; // 定义一个指向输出字符串的指针
    va_start(ap, fmt); // 初始化 va_list 变量，使它指向第一个可变参数
    while (len<n-1) { // 遍历格式字符串
	int length=0;
        if (*fmt == '%') { // 如果遇到 % 符号，表示后面跟着一个格式码
            fmt++; // 移动到格式码的位置
            switch (*fmt) { // 判断格式码的类型
                case 'd': { // 如果是整数类型
                    int d = va_arg(ap, int); // 从可变参数列表中取出一个 int 类型的参数
                    //len += sprintf(p, "%d", d);
                    length = kint2str(d,p); // 将整数格式化为字符串，并写入输出字符串中，同时更新长度变量
		    len+=length;
                    p += length; // 移动指针到输出字符串的末尾
		    while(len>=n){
		      p--;
		      *p='\0';
		      len--;
		    }
                    break;
                }
                case 's': { // 如果是字符串类型
                    char *s = va_arg(ap, char *); // 从可变参数列表中取出一个 char * 类型的参数
                    //len += sprintf(p, "%s", s); // 将字符串复制到输出字符串中，同时更新长度变量
                    int i=0;
                    while(s[i]!='\0'){
                        p[i]=s[i];
                        i++;
                    }
                    length=i;
                    len+=length;
                    p += length; // 移动指针到输出字符串的末尾
		    while(len>=n){
		      p--;
		      *p='\0';
		      len--;
		    } 
                    break;
                }
                case 'c': { // 如果是字符类型
                    char c = (char) va_arg(ap, int); // 从可变参数列表中取出一个 int 类型的参数，并强制转换为 char 类型
                    *p++ = c; // 将字符写入输出字符串中
                    len++; // 更新长度变量
                    break;
                }
                // 其他格式码可以类似地处理
                default: { // 如果是无效的格式码
                    *p++ = '%'; // 将 % 符号写入输出字符串中
                    *p++ = *fmt; // 将格式码写入输出字符串中
                    len += 2; // 更新长度变量
                    break;
                }
            }
        } else { // 如果不是 % 符号，表示是普通的字符
            *p++ = *fmt; // 将字符写入输出字符串中
            len++; // 更新长度变量
        }
        fmt++; // 移动到下一个字符的位置
    }
    *p = '\0'; // 在输出字符串的末尾添加空字符
    va_end(ap); // 结束可变参数的获取
    return len; // 返回输出字符串的长度

}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  //panic("Not implemented");
    int len=0;
    char *p=out;
 while (len<n-1) { // 遍历格式字符串
	int length=0;
        if (*fmt == '%') { // 如果遇到 % 符号，表示后面跟着一个格式码
            fmt++; // 移动到格式码的位置
            switch (*fmt) { // 判断格式码的类型
                case 'd': { // 如果是整数类型
                    int d = va_arg(ap, int); // 从可变参数列表中取出一个 int 类型的参数
                    //len += sprintf(p, "%d", d);
                    length = kint2str(d,p); // 将整数格式化为字符串，并写入输出字符串中，同时更新长度变量
		    len+=length;
                    p += length; // 移动指针到输出字符串的末尾
		    while(len>=n){
		      p--;
		      *p='\0';
		      len--;
		    }
                    break;
                }
                case 's': { // 如果是字符串类型
                    char *s = va_arg(ap, char *); // 从可变参数列表中取出一个 char * 类型的参数
                    //len += sprintf(p, "%s", s); // 将字符串复制到输出字符串中，同时更新长度变量
                    int i=0;
                    while(s[i]!='\0'){
                        p[i]=s[i];
                        i++;
                    }
                    length=i;
                    len+=length;
                    p += length; // 移动指针到输出字符串的末尾
		    while(len>=n){
		      p--;
		      *p='\0';
		      len--;
		    } 
                    break;
                }
                case 'c': { // 如果是字符类型
                    char c = (char) va_arg(ap, int); // 从可变参数列表中取出一个 int 类型的参数，并强制转换为 char 类型
                    *p++ = c; // 将字符写入输出字符串中
                    len++; // 更新长度变量
                    break;
                }
                // 其他格式码可以类似地处理
                default: { // 如果是无效的格式码
                    *p++ = '%'; // 将 % 符号写入输出字符串中
                    *p++ = *fmt; // 将格式码写入输出字符串中
                    len += 2; // 更新长度变量
                    break;
                }
            }
        } else { // 如果不是 % 符号，表示是普通的字符
            *p++ = *fmt; // 将字符写入输出字符串中
            len++; // 更新长度变量
        }
        fmt++; // 移动到下一个字符的位置
    }
    *p = '\0'; // 在输出字符串的末尾添加空字符
    return len;
}

#endif
