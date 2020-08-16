# Minimal example cpprestsdk

Minimal working example testing memory usage when using `web::http::experimental::listener::http_listener` from [cpprestsdk](https://github.com/microsoft/cpprestsdk).

Note on building cpprestsdk with pplx and asio listener:

```bat
cmake ..\Release -A x64 -DBOOST_ROOT=%BOOST% -DBoost_NO_BOOST_CMAKE=ON -DBoost_NO_SYSTEM_PATHS=ON -DBOOST_LIBRARYDIR=%BOOST%\lib\x64\lib -DBoost_USE_STATIC_LIBS=ON -DOPENSSL_ROOT_DIR=%cd%\..\..\openssl\stage\release -DZLIB_INCLUDE_DIR=%cd%\..\..\zlib -DZLIB_LIBRARY=%cd%\..\..\zlib\contrib\vstudio\vc14\x64\ZlibStatReleaseWithoutAsm\zlibstat.lib -DCPPREST_HTTP_LISTENER_IMPL=asio -DCPPREST_PPLX_IMPL=winpplx
```
