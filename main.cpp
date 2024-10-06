#include "inplace_vector.cpp"
#include <iostream>

void printVec(auto&& v){
  for(auto& i: v){
    std::cout << i << ' ';
  }
  std::cout << std::endl;
}

int main(){
  inplace_vector<int, 50> var{0, 1, 2, 3, 4}, toSwap{5, 10, 15, 20};
  var.insert(var.data() + 1, 10);
  var.insert(var.data() + 3, std::move(20));
  var.insert(var.data() + 4, 3, 9);
  var.insert(var.data() + 1, std::initializer_list<int>{11, 12, 13});
  printVec(var);
  for(auto i = var.crbegin(); i != var.crend(); i++){
    std::cout << *i << ' ';
  }
  std::cout << '\n';
  std::array<int, 5> to_insert{91, 92, 93, 94, 95};
  var.insert_range(var.begin() + 4, to_insert);
  printVec(var);
  var.emplace(var.begin() + 1, 3);
  var.try_emplace_back(300);
  var.append_range(to_insert);
  var.try_append_range(to_insert);
  printVec(var);
  var.resize(3);
  var.erase(var.begin() + 1, var.begin() + 2);
  printVec(var);
  var.push_back(3);
  var.push_back(10);
  printVec(var);
  var.swap(toSwap);
  printVec(var);
  printVec(toSwap);
  auto copy = var;
  printVec(copy);
}
