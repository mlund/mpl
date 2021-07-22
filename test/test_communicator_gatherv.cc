#define BOOST_TEST_MODULE communicator_gatherv

#include <boost/test/included/unit_test.hpp>
#include <mpl/mpl.hpp>
#include <tuple>


template<typename T>
bool gatherv_test(const T &val) {
  const mpl::communicator &comm_world{mpl::environment::comm_world()};
  const int N{(comm_world.size() * comm_world.size() + comm_world.size()) / 2};
  std::vector<T> v1(N), v2(N);
  std::iota(begin(v1), end(v1), val);
  mpl::layouts<T> l;
  for (int i{0}, i_end{comm_world.size()}, offset{0}; i < i_end; ++i) {
    l.push_back(mpl::indexed_layout<T>({{i + 1, offset}}));
    offset += i + 1;
  }
  if (comm_world.rank() == 0) {
    comm_world.gatherv(0, v1.data(), l[0], v2.data(), l);
    return v1 == v2;
  } else {
    comm_world.gatherv(0, v1.data(), l[comm_world.rank()]);
    return true;
  }
}


template<typename T>
bool igatherv_test(const T &val) {
  const mpl::communicator &comm_world{mpl::environment::comm_world()};
  const int N{(comm_world.size() * comm_world.size() + comm_world.size()) / 2};
  std::vector<T> v1(N), v2(N);
  std::iota(begin(v1), end(v1), val);
  mpl::layouts<T> l;
  for (int i{0}, i_end{comm_world.size()}, offset{0}; i < i_end; ++i) {
    l.push_back(mpl::indexed_layout<T>({{i + 1, offset}}));
    offset += i + 1;
  }
  if (comm_world.rank() == 0) {
    auto r{comm_world.igatherv(0, v1.data(), l[0], v2.data(), l)};
    r.wait();
    return v1 == v2;
  } else {
    auto r{comm_world.igatherv(0, v1.data(), l[comm_world.rank()])};
    r.wait();
    return true;
  }
}


struct tuple {
  int a;
  double b;
  tuple &operator++() {
    ++a;
    ++b;
    return *this;
  }
};

bool operator==(const tuple&t1, const tuple &t2)
{
  return t1.a== t2.a and t1.b == t2.b;
}

MPL_REFLECTION(tuple, a, b)


BOOST_AUTO_TEST_CASE(gatherv) {
  BOOST_TEST(gatherv_test(1.0));
  BOOST_TEST(gatherv_test(tuple{1, 2.0}));

  BOOST_TEST(igatherv_test(1.0));
  BOOST_TEST(igatherv_test(tuple{1, 2.0}));
}