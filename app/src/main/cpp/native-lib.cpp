/**
 * Generated by FakerAndroid for doc or help by https://github.com/Efaker/FakerAndroid
 */
#include <jni.h>
#include <string>
#include <unistd.h>
#include "curl/curl.h"
#include "public.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"
#include <android/log.h>

#include <curl/curl.h>
mbedtls_x509_crt g_crt;

static void my_debug( void *ctx, int level,
                      const char *file, int line,
                      const char *str )
{
    ((void) level);

    __android_log_print( ANDROID_LOG_ERROR, "HxBreak", "%s:%04d: %s", file, line, str );
}

static size_t writefunction(void *ptr, size_t size, size_t nmemb, void *stream)
{
    fwrite(ptr, size, nmemb, (FILE *)stream);
    return (nmemb*size);
}

static CURLcode sslctx_function(CURL *curl, void *sslctx, void *parm)
{
    CURLcode rv = CURLE_ABORTED_BY_CALLBACK;
    auto ctx = reinterpret_cast<mbedtls_ssl_config *>(sslctx);
    mbedtls_ssl_conf_ca_chain(ctx, &g_crt, NULL);
    rv = CURLE_OK;
    return rv;
}

int test(void)
{
    CURL *ch;
    CURLcode rv;

    curl_global_init(CURL_GLOBAL_ALL);
    ch = curl_easy_init();
    curl_easy_setopt(ch, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(ch, CURLOPT_HEADER, 0L);
    curl_easy_setopt(ch, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(ch, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(ch, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(ch, CURLOPT_SSL_VERIFYHOST, 1L);
    curl_easy_setopt(ch, CURLOPT_URL, "https://www.baidu.com/");


    /* second try: retrieve page using cacerts' certificate -> will succeed
     * load the certificate by installing a function doing the necessary
     * "modifications" to the SSL CONTEXT just before link init
     */
    curl_easy_setopt(ch, CURLOPT_SSL_CTX_FUNCTION, sslctx_function);
    rv = curl_easy_perform(ch);
    __android_log_print(ANDROID_LOG_ERROR, "HxBreak", "ErrorCode: %d", rv);
    if(rv == CURLE_OK)
        printf("*** transfer succeeded ***\n");
    else
        printf("*** transfer failed ***\n");

    curl_easy_cleanup(ch);
    curl_global_cleanup();
    return rv;
}


extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_hookdemo_MainActivity_stringFromJNI(JNIEnv *env, jobject thiz) {
    mbedtls_x509_crt_init(&g_crt);
    auto ret = mbedtls_x509_crt_parse(&g_crt, cacert, cacert_len);
    if (ret < 0 || test() != CURLE_OK){
        return env->NewStringUTF("ERROR");
    }else{
        return env->NewStringUTF("OK");
    }
}