/* Experimental macOS ELAN capture + upstream elanpress matching.
 * Diagnostic only: templates stay in memory, no password or unlock code. */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include "libusb.h"
#include "elanpress-match.h"
static void (*capture_status)(const char*)=NULL;
static void progress(const char*s){puts(s);if(capture_status)capture_status(s);}
static volatile sig_atomic_t cancelled=0;
static void interrupt(int s){cancelled=1;}
typedef struct{libusb_context*ctx;libusb_device_handle*h;int claimed,w,hgt;unsigned char*raw;unsigned short*bg;} Sensor;
static int io(Sensor*s,unsigned char ep,unsigned char*b,int len){int n=0;int r=libusb_bulk_transfer(s->h,ep,b,len,&n,3000);if(r||n!=len){fprintf(stderr,"USB通信失败 %02x: %s (%d/%d)\n",ep,libusb_error_name(r),n,len);return 0;}return 1;}
static int command(Sensor*s,unsigned a,unsigned b){unsigned char c[2]={a,b};return io(s,1,c,2);}
static int frame(Sensor*s,unsigned short*out){unsigned char status;if(cancelled)return 0;if(!command(s,0x40,0x13)||!io(s,0x83,&status,1)||!command(s,0,9)||!io(s,0x82,s->raw,s->w*s->hgt*2))return 0;elanpress_rotate_frame(s->raw,out,s->w,s->hgt);usleep(80000);return !cancelled;}
static void close_sensor(Sensor*s){if(s->h&&s->claimed){command(s,0,11);libusb_release_interface(s->h,0);}if(s->h)libusb_close(s->h);if(s->ctx)libusb_exit(s->ctx);free(s->raw);free(s->bg);}
static int open_sensor(Sensor*s){unsigned char dim[4];if(libusb_init(&s->ctx))return 0;s->h=libusb_open_device_with_vid_pid(s->ctx,0x04f3,0x0c6e);if(!s->h){puts("无法打开 ELAN 04f3:0c6e");return 0;}int r=libusb_claim_interface(s->h,0);if(r){printf("无法占用接口: %s\n",libusb_error_name(r));return 0;}s->claimed=1;if(!command(s,0,12)||!io(s,0x83,dim,4))return 0;s->w=dim[2];s->hgt=dim[0];if(s->w%2&&s->hgt%2){s->w++;s->hgt++;}if(s->w!=150||s->hgt!=52){printf("未验证的尺寸 %dx%d，停止\n",s->w,s->hgt);return 0;}s->raw=calloc(s->w*s->hgt,2);s->bg=calloc(s->w*s->hgt,2);return s->raw&&s->bg&&command(s,0,11);}
static int calibrate(Sensor*s){int size=s->w*s->hgt;unsigned short*tmp=calloc(size,2);unsigned long*sum=calloc(size,sizeof(unsigned long));if(!tmp||!sum){free(tmp);free(sum);return 0;}progress("请把手指移开，正在采集背景…");for(int i=0;i<12;i++){if(!frame(s,tmp)){free(tmp);free(sum);return 0;}for(int j=0;j<size;j++)sum[j]+=tmp[j];}for(int j=0;j<size;j++)s->bg[j]=sum[j]/12;free(tmp);free(sum);return 1;}
/* Require clear sensor before each new capture; 4 low frames debounce lift. */
static ElanpressFeatures*capture(Sensor*s){int size=s->w*s->hgt;unsigned short*tmp=calloc(size,2);GSList*list=NULL;int n=0,clear=0,started=0;time_t deadline=time(NULL)+90;ElanpressFeatures*f=NULL;if(!tmp)return NULL;
 while(!cancelled&&time(NULL)<deadline){if(!frame(s,tmp))break;ElanpressTouchStats stats;int touched=elanpress_frame_has_touch(tmp,s->bg,size,&stats);
  if(!started){if(!touched){clear++;if(clear==4){progress("现在请轻触指纹区域并保持，采集后会提示松开。");fflush(stdout);}continue;}if(clear<4){clear=0;continue;}started=1;}
  if(!touched)continue;
  GSList*node=calloc(1,sizeof(*node));if(!node)break;node->data=malloc(size*2);if(!node->data){free(node);break;}memcpy(node->data,tmp,size*2);node->next=list;list=node;n++;
  if(n>=10){unsigned char*img=elanpress_process_frames(list,n,s->bg,size);if(img){double quality=elanpress_image_quality(img,size);f=elanpress_image_features(img,s->w,s->hgt);printf("采集完成：图像对比度 %.1f，特征点 %u。请松开。\n",quality,f?f->n:0);progress("采集完成，请松开手指。");if(quality<ELANPRESS_MIN_QUALITY_STDDEV||!f||f->n<ELANPRESS_MATCH_MIN_PAIRS){puts("图像质量不足，本次请重试。");if(f)elanpress_features_free(f);f=NULL;}memset(img,0,size);free(img);}break;}
 }
 while(list){GSList*next=list->next;memset(list->data,0,size*2);free(list->data);free(list);list=next;}memset(tmp,0,size*2);free(tmp);return f;}
static unsigned match(ElanpressFeatures*f,ElanpressFeatures**templates,int n){unsigned best=0;for(int i=0;i<n;i++){unsigned score=elanpress_match_features(f,templates[i]);if(score>best)best=score;}return best;}
static int pause_prompt(const char*msg){puts(msg);fflush(stdout);int c;do{c=getchar();if(c==EOF)return 0;}while(c!='\n');return !cancelled;}
static int self_test(void){unsigned char a[150*52],b[150*52],blank[150*52]={0};uint32_t x=13,y=999;for(int i=0;i<150*52;i++){x=x*1664525u+1013904223u;y=y*1664525u+1013904223u;a[i]=x>>24;b[i]=y>>24;}ElanpressFeatures*fa=elanpress_image_features(a,150,52),*fb=elanpress_image_features(b,150,52),*fz=elanpress_image_features(blank,150,52);unsigned same=elanpress_match_features(fa,fa),other=elanpress_match_features(fa,fb),zero=elanpress_match_features(fa,fz);printf("Synthetic matcher check: points=%u same=%u unrelated=%u blank=%u\n",fa->n,same,other,zero);int ok=fa->n>=12&&same>=12&&other<12&&zero==0;elanpress_features_free(fa);elanpress_features_free(fb);elanpress_features_free(fz);return ok?0:1;}
int main(int argc,char**argv){if(argc==2&&!strcmp(argv[1],"--self-test"))return self_test();setbuf(stdout,NULL);signal(SIGINT,interrupt);signal(SIGTERM,interrupt);Sensor s={0};ElanpressFeatures*t[8]={0};int rc=1;
 puts("ELAN 指纹录入与匹配实验\n模板仅在本次运行内存中保存。按 Control+C 可以停止。");if(!pause_prompt("先把手指移开，按回车开始背景采集。"))goto end;if(!open_sensor(&s)||!calibrate(&s))goto end;
 puts("第一阶段：使用同一根手指，录入 8 次。每次采集后松开，下一次稍微改变角度。");for(int i=0;i<8;i++){printf("\n录入 %d/8\n",i+1);for(int tries=0;tries<3&&!t[i]&&!cancelled;tries++)t[i]=capture(&s);if(!t[i])goto end;}
 if(!pause_prompt("\n第二阶段：仍然用刚才那根手指，验证 3 次。准备好后按回车。"))goto end;int genuine=0,impostor=0;
 for(int i=0;i<3;i++){printf("\n同指测试 %d/3\n",i+1);ElanpressFeatures*f=capture(&s);if(!f)goto end;unsigned score=match(f,t,8);printf("匹配特征对：%u；参考阈值 %d；%s\n",score,ELANPRESS_MATCH_MIN_PAIRS,score>=ELANPRESS_MATCH_MIN_PAIRS?"判为匹配":"判为不匹配");if(score>=ELANPRESS_MATCH_MIN_PAIRS)genuine++;elanpress_features_free(f);}
 if(!pause_prompt("\n第三阶段：换成没有录入的其他手指，验证 3 次。准备好后按回车。"))goto end;
 for(int i=0;i<3;i++){printf("\n其他手指测试 %d/3\n",i+1);ElanpressFeatures*f=capture(&s);if(!f)goto end;unsigned score=match(f,t,8);printf("匹配特征对：%u；%s\n",score,score>=ELANPRESS_MATCH_MIN_PAIRS?"出现误接受":"判为不匹配");if(score>=ELANPRESS_MATCH_MIN_PAIRS)impostor++;elanpress_features_free(f);}
 printf("\n实验结果：同指接受 %d/3；其他手指误接受 %d/3。\n这只是初步实验，不能据此证明解锁安全性。\n",genuine,impostor);rc=0;
end:for(int i=0;i<8;i++)if(t[i])elanpress_features_free(t[i]);close_sensor(&s);puts("本次内存模板已释放。没有修改密码或登录配置。");return rc;}
