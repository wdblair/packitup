#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static const char* nginxargs[] = {"nginx", "-p", "/tmp/" ,"-c", "nginx.conf", '\0'};

const char** getpayloadargs(int *len) {
   *len = 5;
   return nginxargs;	
}

char *conf = 
" #user  nobody; \n"
" worker_processes  1; \n"
"  \n"
" error_log  /tmp/error.log; \n"
" error_log  /tmp/error.log  notice; \n"
" error_log  /tmp/error.log  info; \n"
"  \n"
" pid        /tmp/nginx.pid; \n"
"  \n"
" events { \n"
"     worker_connections  1024; \n"
" } \n"
"  \n"
" http { \n"
"  \n"
"     types { \n"
"     text/html                             html htm shtml; \n"
"     text/css                              css; \n"
"     text/xml                              xml; \n"
"     image/gif                             gif; \n"
"     image/jpeg                            jpeg jpg; \n"
"     application/javascript                js; \n"
"     application/atom+xml                  atom; \n"
"     application/rss+xml                   rss; \n"
"  \n"
"     text/mathml                           mml; \n"
"     text/plain                            txt; \n"
"     text/vnd.sun.j2me.app-descriptor      jad; \n"
"     text/vnd.wap.wml                      wml; \n"
"     text/x-component                      htc; \n"
"  \n"
"     image/png                             png; \n"
"     image/tiff                            tif tiff; \n"
"     image/vnd.wap.wbmp                    wbmp; \n"
"     image/x-icon                          ico; \n"
"     image/x-jng                           jng; \n"
"     image/x-ms-bmp                        bmp; \n"
"     image/svg+xml                         svg svgz; \n"
"     image/webp                            webp; \n"
"  \n"
"     application/font-woff                 woff; \n"
"     application/java-archive              jar war ear; \n"
"     application/json                      json; \n"
"     application/mac-binhex40              hqx; \n"
"     application/msword                    doc; \n"
"     application/pdf                       pdf; \n"
"     application/postscript                ps eps ai; \n"
"     application/rtf                       rtf; \n"
"     application/vnd.apple.mpegurl         m3u8; \n"
"     application/vnd.ms-excel              xls; \n"
"     application/vnd.ms-fontobject         eot; \n"
"     application/vnd.ms-powerpoint         ppt; \n"
"     application/vnd.wap.wmlc              wmlc; \n"
"     application/vnd.google-earth.kml+xml  kml; \n"
"     application/vnd.google-earth.kmz      kmz; \n"
"     application/x-7z-compressed           7z; \n"
"     application/x-cocoa                   cco; \n"
"     application/x-java-archive-diff       jardiff; \n"
"     application/x-java-jnlp-file          jnlp; \n"
"     application/x-makeself                run; \n"
"     application/x-perl                    pl pm; \n"
"     application/x-pilot                   prc pdb; \n"
"     application/x-rar-compressed          rar; \n"
"     application/x-redhat-package-manager  rpm; \n"
"     application/x-sea                     sea; \n"
"     application/x-shockwave-flash         swf; \n"
"     application/x-stuffit                 sit; \n"
"     application/x-tcl                     tcl tk; \n"
"     application/x-x509-ca-cert            der pem crt; \n"
"     application/x-xpinstall               xpi; \n"
"     application/xhtml+xml                 xhtml; \n"
"     application/xspf+xml                  xspf; \n"
"     application/zip                       zip; \n"
"  \n"
"     application/octet-stream              bin exe dll; \n"
"     application/octet-stream              deb; \n"
"     application/octet-stream              dmg; \n"
"     application/octet-stream              iso img; \n"
"     application/octet-stream              msi msp msm; \n"
"  \n"
"     application/vnd.openxmlformats-officedocument.wordprocessingml.document    docx; \n"
"     application/vnd.openxmlformats-officedocument.spreadsheetml.sheet          xlsx; \n"
"     application/vnd.openxmlformats-officedocument.presentationml.presentation  pptx; \n"
"  \n"
"     audio/midi                            mid midi kar; \n"
"     audio/mpeg                            mp3; \n"
"     audio/ogg                             ogg; \n"
"     audio/x-m4a                           m4a; \n"
"     audio/x-realaudio                     ra; \n"
"  \n"
"     video/3gpp                            3gpp 3gp; \n"
"     video/mp2t                            ts; \n"
"     video/mp4                             mp4; \n"
"     video/mpeg                            mpeg mpg; \n"
"     video/quicktime                       mov; \n"
"     video/webm                            webm; \n"
"     video/x-flv                           flv; \n"
"     video/x-m4v                           m4v; \n"
"     video/x-mng                           mng; \n"
"     video/x-ms-asf                        asx asf; \n"
"     video/x-ms-wmv                        wmv; \n"
"     video/x-msvideo                       avi; \n"
"     } \n"
"     default_type  application/octet-stream; \n"
"  \n"
"     access_log  off; \n"
"     error_log off; \n"
"  \n"
"     sendfile        on; \n"
"     #tcp_nopush     on; \n"
"  \n"
"     #keepalive_timeout  0; \n"
"     keepalive_timeout  65; \n"
"  \n"
"     #gzip  on; \n"
"  \n"
"     server { \n"
"         listen       8080; \n"
"         server_name  localhost; \n"
"  \n"
"         #charset koi8-r; \n"
"  \n"
" 	rewrite ^/search/(.*)$ http://google.com/?q=$1 break; \n"
"  \n"
"         root /home/; \n"
"  \n"
"         location / { \n"
"             autoindex on; \n"
"         } \n"
"  \n"
"         #error_page  404              /404.html; \n"
"  \n"
"     } \n"
"  \n"
" } \n";

int main (int argc, char *argv[]) {
   /**

   */    
   fprintf(stdout, "Setting up nginx!\n");
 
   system("mkdir -p /tmp/logs");
 
   FILE *fconf = fopen("/tmp/nginx.conf", "w"); 

   if(!fconf) {
       fprintf(stderr, "Could not create nginx config file!");
       exit(1); 
   }
   
   int written = fwrite(conf, sizeof(char), strlen(conf), fconf);

   if(written != strlen(conf)) {
       fprintf(stderr, "Could not write all log file!\n"); 
   }

   fclose(fconf);
}
