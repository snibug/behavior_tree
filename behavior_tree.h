//
// 2016 write snibug
//

#ifndef BEHAVIOR_TREE_BEHAVIOR_TREE_H_
#define BEHAVIOR_TREE_BEHAVIOR_TREE_H_

#define EXPORT

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
TypeName(const TypeName&);                 \
void operator=(const TypeName&)

#include <vector>
#include <memory>

class EXPORT Task {
 public:
  enum Status {
    STATUS_NONE = 0,
    STATUS_SUCCESS = 1,
    STATUS_FAILURE = 2,
    STATUS_RUNNING = 3,
    STATUS_ABORTED = 4,
  };

  Task();
  virtual ~Task();

  virtual int Update() = 0;

  virtual void OnStart();
  virtual void OnEnd();

  int Tick();

  void Reset();

  void Abort();

  bool IsEnd();

  bool IsRunning();

  int status() {
    return status_;
  }

 private:
  int status_;

  DISALLOW_COPY_AND_ASSIGN(Task);
};

class EXPORT Composit : public Task {
 public:
  Composit();
  ~Composit() override;

  void OnStart() override;
  void OnEnd() override;

  void AddChild(Task* child);
  void RemoveChild(int pos);
  void ClearChilden();

 protected:
  typedef std::vector<std::unique_ptr<Task>> Tasks;

  // current task
  Task* current();

  // get first task
  Task* begin();

  // get next task if next is not exist return null pointer
  Task* next();

  int size() {
    return static_cast<int>(tasks_.size());
  }

 private:
  Tasks tasks_;
  int current_pos_;

  DISALLOW_COPY_AND_ASSIGN(Composit);
};

class EXPORT Sequence : public Composit {
 public:
  Sequence();
  ~Sequence() override;

  int Update() override;

 private:
  DISALLOW_COPY_AND_ASSIGN(Sequence);
};

class EXPORT Selector : public Composit {
 public:
  Selector();
  ~Selector() override;

  int Update() override;

 private:
  DISALLOW_COPY_AND_ASSIGN(Selector);
};

class EXPORT Decorator : public Task {
 public:
  Decorator();
  ~Decorator() override;

  Task* child() {
    return child_.get();
  }

  void set_child(Task* child) {
    child_.reset(child);
  }

 private:
  std::unique_ptr<Task> child_;

  DISALLOW_COPY_AND_ASSIGN(Decorator);
};

class EXPORT Repeat : public Decorator {
 public:
  Repeat();
  ~Repeat() override;

  void OnStart() override;
  void OnEnd() override;

  int Update() override;

  void set_cout(int count) {
    limit_ = count;
  }

 private:
  int limit_;
  int count_;

  DISALLOW_COPY_AND_ASSIGN(Repeat);
};

class EXPORT AlwaysFail : public Decorator {
 public:
  AlwaysFail();
  ~AlwaysFail() override;

  int Update() override;

 private:
  DISALLOW_COPY_AND_ASSIGN(AlwaysFail);
};

class EXPORT AlwaysSuccess : public Decorator {
 public:
  AlwaysSuccess();
  ~AlwaysSuccess() override;

  int Update() override;

 private:
  DISALLOW_COPY_AND_ASSIGN(AlwaysSuccess);
};

class EXPORT Invert : public Decorator {
 public:
  Invert();
  ~Invert() override;

  int Update() override;

 private:
  DISALLOW_COPY_AND_ASSIGN(Invert);
};

class UntilSuccess : public Decorator {
 public:
  UntilSuccess();
  ~UntilSuccess();

  int Update() override;

 private:
  DISALLOW_COPY_AND_ASSIGN(UntilSuccess);
};

class UntilFail : public Decorator {
 public:
  UntilFail();
  ~UntilFail() override;

  int Update() override;

 private:
  DISALLOW_COPY_AND_ASSIGN(UntilFail);
};

class EXPORT Parallel : public Composit {
 public:
  enum Policy {
    POLICY_NONE = 0,
    REQUIRE_ONE = 1,
    REQUIRE_ALL = 2,
  };

  Parallel();
  ~Parallel() override;

  void OnStart() override;
  void OnEnd() override;

  int Update() override;

  void set_policy(Policy success, Policy failure);

  int success_policy() {
    return success_policy_;
  }

  int failure_policy() {
    return failure_policy_;
  }

 private:
  int success_policy_;
  int failure_policy_;

  DISALLOW_COPY_AND_ASSIGN(Parallel);
};

#endif  // BEHAVIOR_TREE_BEHAVIOR_TREE_H_
