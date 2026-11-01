#pragma once
#include "SaveTransfer.hpp"
#include <filesystem>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
namespace heavymach::transfer {
inline std::string readFile(const std::filesystem::path&p){std::ifstream f(p,std::ios::binary);if(!f)throw std::runtime_error("Could not read recovery file");std::string s((std::istreambuf_iterator<char>(f)),{});if(s.size()>24000)throw std::runtime_error("Recovery file too large");return s;}
inline void atomicFile(const std::filesystem::path&p,std::string_view value){
 auto tmp=p;tmp+=".pending";std::filesystem::create_directories(p.parent_path());
 int fd=::open(tmp.c_str(),O_WRONLY|O_CREAT|O_TRUNC,0600);if(fd<0)throw std::runtime_error("Could not create recovery file");size_t n=0;
 while(n<value.size()){auto written=::write(fd,value.data()+n,value.size()-n);if(written<=0){::close(fd);throw std::runtime_error("Could not write recovery file");}n+=size_t(written);}int result=::fsync(fd);::close(fd);if(result<0)throw std::runtime_error("Could not finish recovery file");std::filesystem::rename(tmp,p);
 fd=::open(p.parent_path().c_str(),O_RDONLY);if(fd<0)throw std::runtime_error("Could not open recovery directory");result=::fsync(fd);::close(fd);if(result<0)throw std::runtime_error("Could not finish recovery directory");
}
// Native file I/O adapter. Game formats remain owned by each existing platform.
class FileStore final:public Store {
 std::filesystem::path root_;std::function<Snapshot()> read_;std::function<void(const Snapshot&)> apply_;
public:
 FileStore(std::filesystem::path root,std::function<Snapshot()>r,std::function<void(const Snapshot&)>a):root_(std::move(root)),read_(std::move(r)),apply_(std::move(a)){}
 Snapshot read()override{return read_();}
 void preserve(std::string_view code)override{atomicFile(root_/"before.hmcode",code);}
 void pending(std::string_view code)override{atomicFile(root_/"restore.hmcode",code);}
 void apply(const Snapshot&s)override{apply_(s);}
 void finish()override{std::filesystem::remove(root_/"restore.hmcode");}
 void recover(){auto p=root_/"restore.hmcode";if(std::filesystem::exists(p))resume(*this,readFile(p));}
};
}
