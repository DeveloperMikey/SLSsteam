library-inject.so: main.cpp
	#g++ main.cpp -O3 -m32 -fPIC -shared -std=c++20 -o library-inject.so
	#Disabled, since having a la_objsearch fucks up exceptions for some reason
	rm "library-inject.so"
	touch "library-inject.so"
