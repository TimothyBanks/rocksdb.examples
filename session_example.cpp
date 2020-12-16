#include <rocksdb/db.h>
#include <session/rocks_session.hpp>
#include <session/session.hpp>
#include <session/undo_stack.hpp>

#include <memory>

using rocks_db_type = eosio::session::session<eosio::session::rocksdb_t>;
using session_type = eosio::session::session<rocks_db_type>;

auto make_rocksdb_db() -> std::unique_ptr<rocks_db_type> {
  auto options = rocksdb::Options{};
  options.create_if_missing = true;
  rocksdb::DB* p{nullptr};
  auto status = rocksdb::DB::Open(options, ".", &p);
  if (!status.ok()) {
    throw std::runtime_error(std::string{"database::database: rocksdb::DB::Open: "} + status.ToString());
  }
  auto rdb = std::shared_ptr<rocksdb::DB>{p};
  return std::make_unique<rocks_db_type>(eosio::session::make_session(std::move(rdb), 1024));
}

int main(int argc, char *argv[]) { 
  // Create a session to the rocksdb data store.
  auto root_session = make_rocksdb_db(); 

  // Create an undo_stack for managing the sessions that will potentially commit their data into the rocksdb datastore.
  // The undo_stack isn't necessary for this workflow.  The alternative approach involves the fact that the sessions
  // maintain a parent-child relationship.
  // root_sssion->write(...); // Write something directly into rocksdb.
  // auto child_session = eosio::session::session{root_session, nullptr}; // Create a child session
  // child_session->write(...); // Write something into the in-memory session.
  // child_session->commit(); // Will commit changes into the root_session.
  // // You can create a linked list of child sessions if you want.
  // auto child_session_2 = eosio::session::session{child_session, nullptr};
  // child_session_2->write(...); // Write something into the in-memory session.
  // child_session_2->commit(); // Commit the changes up into child_session (the parent).
  auto undo_stack = eosio::session::undo_stack<rocks_db_type>{*root_session};

  auto print_value = [](auto& session, const auto& key) {
    auto value = session.read(key);
    if (value) {
      std::cout << "{key, value} = {" << key << ", " << *value << "}" << std::endl;
    }
  };

  auto write_value = [&](const char* key_data, const char* value_data) {
    auto top = undo_stack.top();
    std::visit([&](auto* session){
      auto key = eosio::session::shared_bytes{key_data, strlen(key_data)};
      auto value = eosio::session::shared_bytes{value_data, strlen(value_data)};
      session->write(key, value);
      print_value(*session, key);
    }, top.holder());
  };

  // We can write directly into the rocksdb datastore when there are no sessions on the undo stack.
  // Changes written into this session can only be undone by manually erasing through the session
  // or by manually rolling back changes to any key-value pairs.  Normally with an in memory session, 
  // you could call undo on the session
  write_value("foo1", "hello world");

  // Push a new session onto the stack.
  undo_stack.push();

  // We can write into that newly pushed session.
  write_value("foo2", "hello again");

  // When you are done, you can choose to commit or undo the changes.
  // undo_stack.undo();
  undo_stack.commit(undo_stack.revision());
  // Since this is the only session on the stack, you could also squash
  // which merges the top two sessions on the undo stack together.
  // undo_stack.squash();

  auto top = undo_stack.top();  
  std::visit([&](auto* session){
    auto key = eosio::session::shared_bytes{"foo1", 4};
    print_value(*session, key);
    key = eosio::session::shared_bytes{"foo2", 4};
    print_value(*session, key);
  }, top.holder());

  // Iteration is also supported by getting a reference to a session instance.
  // begin/end/find/lower_bound are supported operations.
  // forward and backward iteration supported.

  return 0; 
}