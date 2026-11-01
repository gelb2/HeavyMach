#pragma once
#include "../../Sources/Core/SaveData.hpp"
#include "../../Sources/Core/HUDConfiguration.hpp"
#include "../../Sources/Core/KeyboardInput.hpp"
#include <functional>
#include <optional>
#include <vector>
namespace heavymach::transfer {
struct Snapshot {
 SaveData game;
 std::array<std::optional<hud::Placement>,hud::groupCount> positions{};
 std::optional<std::array<unsigned,KeyboardInput::count>> keys=KeyboardInput::defaults;
 int aim=1;
};
void validate(const Snapshot&);
std::string encode(const Snapshot&);
Snapshot decode(std::string_view);
// source/target -1 selects all slots. Other slots are preserved for a single-slot import.
Snapshot merge(const Snapshot& current,const Snapshot& incoming,int source=-1,int target=-1,bool settings=true);
std::string summary(const Snapshot&);
bool equal(const Snapshot&,const Snapshot&);
enum class Step {preserveBefore,writePending,apply,verify,finish,complete};
// Both native and Wasm hosts acknowledge each durable operation before advancing.
class Restore {
 Snapshot before_,after_;Step step_=Step::preserveBefore;
public:
 Restore(Snapshot before,Snapshot after):before_(std::move(before)),after_(std::move(after)){validate(before_);validate(after_);}
 Step step()const{return step_;}
 const Snapshot& before()const{return before_;} const Snapshot& after()const{return after_;}
 void acknowledge(); void verify(const Snapshot&);
};
// Native storage adapters provide I/O only. Pending must be durable before apply.
struct Store {
 virtual ~Store()=default;
 virtual Snapshot read()=0;
 virtual void preserve(std::string_view)=0;
 virtual void pending(std::string_view)=0;
 virtual void apply(const Snapshot&)=0;
 virtual void finish()=0;
};
void restore(Store&,const Snapshot& target);
void resume(Store&,std::string_view pendingCode);
}
