/**
 * Generated by FakerAndroid for doc or help by https://github.com/Efaker/FakerAndroid
 */
#include <jni.h>
#include <string>
#include <unistd.h>
#include "curl/curl.h"
#include "public.h"
#include "mbedtls/ssl.h"
#include "mbedtls/x509.h"
#include <android/log.h>

#include <curl/curl.h>

static size_t writefunction(void *ptr, size_t size, size_t nmemb, void *stream)
{
    fwrite(ptr, size, nmemb, (FILE *)stream);
    return (nmemb*size);
}

static CURLcode sslctx_function(CURL *curl, void *sslctx, void *parm)
{
    CURLcode rv = CURLE_ABORTED_BY_CALLBACK;
    auto ctx = reinterpret_cast<mbedtls_ssl_config *>(sslctx);
    mbedtls_x509_crt crt;
    mbedtls_x509_crt_init(&crt);
    mbedtls_x509_crt_parse(&crt, cacert, cacert_len);
    mbedtls_ssl_conf_ca_chain(ctx, &crt, NULL);
    rv = CURLE_OK;
    return rv;
}

int test(void)
{
    CURL *ch;
    CURLcode rv;

    curl_global_init(CURL_GLOBAL_ALL);
    ch = curl_easy_init();
    curl_easy_setopt(ch, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt(ch, CURLOPT_HEADER, 0L);
    curl_easy_setopt(ch, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(ch, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(ch, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(ch, CURLOPT_SSL_VERIFYHOST, 1L);
    curl_easy_setopt(ch, CURLOPT_URL, "https://www.baidu.com/");

    /* Turn off the default CA locations, otherwise libcurl will load CA
     * certificates from the locations that were detected/specified at
     * build-time
     */
    curl_easy_setopt(ch, CURLOPT_CAINFO, NULL);
    curl_easy_setopt(ch, CURLOPT_CAPATH, NULL);

    /* first try: retrieve page without ca certificates -> should fail
     * unless libcurl was built --with-ca-fallback enabled at build-time
     */
    rv = curl_easy_perform(ch);
    if(rv == CURLE_OK)
        printf("*** transfer succeeded ***\n");
    else
        printf("*** transfer failed ***\n");

    /* use a fresh connection (optional)
     * this option seriously impacts performance of multiple transfers but
     * it is necessary order to demonstrate this example. recall that the
     * ssl ctx callback is only called _before_ an SSL connection is
     * established, therefore it will not affect existing verified SSL
     * connections already in the connection cache associated with this
     * handle. normally you would set the ssl ctx function before making
     * any transfers, and not use this option.
     */
    curl_easy_setopt(ch, CURLOPT_FRESH_CONNECT, 1L);

    /* second try: retrieve page using cacerts' certificate -> will succeed
     * load the certificate by installing a function doing the necessary
     * "modifications" to the SSL CONTEXT just before link init
     */
    curl_easy_setopt(ch, CURLOPT_SSL_CTX_FUNCTION, sslctx_function);
    rv = curl_easy_perform(ch);
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
    if (test() == CURLE_OK){
        return env->NewStringUTF("OK");
    }else{
        return env->NewStringUTF("ERROR");
    }
}