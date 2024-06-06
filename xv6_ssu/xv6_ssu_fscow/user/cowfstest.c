#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "fs.h"

#define TRUE 1
#define FALSE 0

int compare_files(const char *file1, const char *file2);
int compare_buffers(char *buf1, char *buf2, int size);
void cat(int fd);

int main()
{
  char buf[512];
  int fd1, fd2, fd3, blockcnt;
  char *f1 = "og.file";
  char *f2 = "cp1.file";
  char *f3 = "cp2.file";
  int isExist = FALSE;
  int t3_result[3]; /* Save result of sub-tests of TEST3 */

  printf(1, "=== COWFS TEST ===\n");

  /*  TEST1 : cp-cow files & compare each file's data */
  /////////////////////////////////////////////////////////////////////////////////////////
  printf(1, "===== TEST 1 =====\n");
	//printf(1, "opening fd1 for create, rdwr\n");
  fd1 = open(f1, O_CREATE | O_RDWR); 
  if(fd1 < 0){
    printf(1, "open: cannot open og.file\n");
    exit();
  }
  blockcnt = 0;
	//printf(1, "writing fd1\n");
  while(1){
    *(int*)buf = 'a';
    if((write(fd1, buf, sizeof(buf)) <= 0))
      break;
    blockcnt++;
    if(blockcnt % 10 == 0) printf(1, ".");
  }

  printf(1, "\n");
	//printf(1, "closing fd1\n");
  close(fd1);

	//printf(1, "opening fd1 for nothing\n");
  fd1 = open(f1, 0);
  if(fd1 < 0){
    printf(1, "open: cannot open og.file\n");
    exit();
  }

	//printf(1, "opening fd2 for create, rdwr\n");
  fd2 = open(f2, O_CREATE | O_RDWR);
  if(fd2 < 0){ 
    printf(1, "open: cannot cp1.file\n");
    close(fd1); 
    exit();
  }
	
	//printf(1, "coping fd1 to fd2\n");
  if(cpcow(fd1, fd2) <= 0){
    printf(1, "Can't copy all block to cp1.file\n");
    close(fd1);
    close(fd2);
    exit();
  }
	//printf(1, "closing fd1\n");
  close(fd1);
	//printf(1, "closing fd2\n");
  close(fd2);

	//printf(1, "opening fd1 for nothing\n");
  fd1 = open(f1, 0);
  if(fd1 < 0){
    printf(1, "open: cannot open og.file\n");
    exit();
  }
	//printf(1, "opening fd3 for create, rdwr\n");
  fd3 = open(f3, O_CREATE | O_RDWR);
  if(fd3 < 0){ 
    printf(1, "open: cannot cp2.file\n");
    close(fd1); 
    exit();
  }

	
	//printf(1, "coping fd1 to fd3\n");
  if(cpcow(fd1, fd3) <= 0){
    printf(1, "Can't copy all block to cp2.file\n");
    close(fd1);
    close(fd3);
    exit();
  }
	//printf(1, "closing fd1\n");
  close(fd1);
	//printf(1, "closing fd3\n");
  close(fd3);

	//printf(1, "comparing f1, f2, f3(all same)\n");
  if(compare_files(f1, f2) == 1) {
    if(compare_files(f1, f3) == 1) printf(1, "1. OK\n");
    else printf(1, "1. WRONG\n");
  }
  else printf(1, "1. WRONG\n");

  /*  TEST2 : update file & compare each file's data */
  /////////////////////////////////////////////////////////////////////////////////////////
  printf(1, "===== TEST 2 =====\n");
	//printf(1, "opening fd3\n");
  fd3 = open(f3, 2);
  if(fd3 < 0){ 
    printf(1, "open: cannot cp2.file\n");
    exit();
  }
  blockcnt = 0;
	//printf(1, "writing to fd3\n");
  while(1){
    *(int*)buf = 'b';
    if((write(fd3, buf, sizeof(buf)) <= 0))
        break;
    blockcnt++;
    if(blockcnt % 10 == 0) printf(1, ".");
  }
  printf(1, "\n");
	//printf(1, "comparing f1,f2(same), f1,f3(diff)\n");
  if(compare_files(f1, f2) == 1) {
    if(compare_files(f1, f3) == 0) printf(1, "2. OK\n");
    else printf(1, "2. WRONG\n");
  }
  else printf(1, "2. WRONG\n");

  /*  TEST3 : rm file & check remaining file data */
  /////////////////////////////////////////////////////////////////////////////////////////
  printf(1, "===== TEST 3 =====\n");
  unlink(f3);
  if(compare_files(f1, f2) == 1)
  {
    if((fd3=open(f3, 0)) < 0) 
      t3_result[0] = TRUE;
    else t3_result[0] = FALSE;
  }
  else 
    t3_result[0] = FALSE;

  if((fd1 = open(f1, 0)) > 0) 
    isExist = TRUE;
  else
    isExist = FALSE;
  close(fd1);
  unlink(f1);
  if((fd1 = open(f1, 0)) < 0 && isExist){
    t3_result[1] = TRUE;
  }
  else t3_result[1] = FALSE; 

  if((fd2 = open(f2, 0)) > 0) 
    isExist = TRUE;
  else
    isExist = FALSE;
  close(fd2);
  unlink(f2);
  if((fd2 = open(f2, 0)) < 0 && isExist){ 
    t3_result[2] = TRUE;
  }
  else t3_result[2] = FALSE;

  if (t3_result[0] && t3_result[1] && t3_result[2])
    printf(1, "3. OK\n");
  else
    printf(1, "3. WRONG\n");

  /*  TEST4 : [STRESS TEST] make copied files */
  ////////////////////////////////////////////////////////////////////////////////
  printf(1, "===== TEST 4 =====\n");
  char* file[9] = {"1.file", "2.file", "3.file", "4.file", "5.file", "6.file", "7.file", "8.file", "9.file"};
  fd1 = open(f1, O_CREATE | O_RDWR);
  if(fd1 < 0){
    printf(1, "open: cannot open og.file\n");
    exit();
  }
  blockcnt = 0;
  while(1){
    *(int*)buf = 'a';
    if((write(fd1, buf, sizeof(buf)) <= 0))
      break;
    blockcnt++;
    if(blockcnt % 10 == 0) printf(1, ".");
  }
  printf(1, "\n");
  close(fd1);
  for(int a = 0; a < 9; ++a){
    fd1 = open(f1, 2); 
    if(fd1 < 0){
      printf(1, "open: cannot open og.file\n");
      exit();
    }
    fd2 = open(file[a], O_CREATE | O_RDWR); 
    if(fd2 < 0){
      printf(1, "open: cannot open copy file\n");
      exit();
    }
    if(cpcow(fd1, fd2) <= 0){
      printf(1, "Can't copy all block to cp1.file\n");
      close(fd1);
      close(fd2);
      exit();
    }
    close(fd1);
    close(fd2);
    if(compare_files(f1, file[a]) == 1) printf(1, "4-%d) OK\n", a+1);
    else printf(1, "WRONG\n");
  }

  /*  TEST5 : rm files */
  ////////////////////////////////////////////////////////////////////////////////
  printf(1, "===== TEST 5 =====\n");
  printf(1, "   (After TEST 4)\n");
  unlink(f1);
  if((fd1 = open(f1, 0)) < 0)
    printf(1, "og.file is deleted(OK)\n");
  for(int b = 0; b < 9; ++b){
    unlink(file[b]);
    if((fd1 = open(file[b], 0)) < 0)
      printf(1, "%s is deleted(OK)\n", file[b]);
  }
  exit();
}



int compare_buffers(char *buf1, char *buf2, int size) {
    for (int i = 0; i < size; i++) {
        if (buf1[i] != buf2[i]) {
            return 0;
        }
    }
    return 1;
}
int compare_files(const char *file1, const char *file2) {
    int fd1, fd2;
    int n1, n2;
    char buf1[140], buf2[140];
    fd1 = open(file1, O_RDONLY);
    if (fd1 < 0) {
      printf(1, "cannot open %s\n", file1);
      return -1;
    }
    fd2 = open(file2, O_RDONLY);
    if (fd2 < 0) {
      printf(1, "cannot open %s\n", file2);
      close(fd1);
      return -1;
    }
    while (1) {
      n1 = read(fd1, buf1, sizeof(buf1));
      n2 = read(fd2, buf2, sizeof(buf2));
      if (n1 < 0 || n2 < 0) {
        printf(1, "error reading files\n");
        printf(1, "can't read\n");
        close(fd1);
        close(fd2);
        return -1;
      }
      int tmp = compare_buffers(buf1, buf2, n1);
      if (n1 != n2 || tmp == 0) {
        close(fd1);
        close(fd2);
        return 0;
      }
      if (n1 == 0 && n2 == 0) {
        break;
      }
    }
    close(fd1);
    close(fd2);
    return 1;
}
void
cat(int fd)
{
  int n;
  char buf[512];
  while((n = read(fd, buf, sizeof(buf))) > 0) {
    if (write(1, buf, n) != n) {
      printf(1, "cat: write error\n");
      exit();
    }
  }
  if(n < 0){
    printf(1, "cat: read error\n");
    exit();
  }
  printf(1,"\n");
}
