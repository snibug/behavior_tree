//
// 2016 write snibug
//

#include "behavior_tree.h"

Task::Task()
    : status_(STATUS_NONE) {
}

Task::~Task() {}

void Task::OnStart() {}

void Task::OnEnd() {}

int Task::Tick() {
  if (status_ == STATUS_NONE) {
    OnStart();
  }

  status_ = Update();

  if (status_ != STATUS_RUNNING) {
    OnEnd();
  }

  return status_;
}

void Task::Reset() {
  status_ = STATUS_NONE;
}

void Task::Abort() {
  status_ = STATUS_ABORTED;
  OnEnd();
}

bool Task::IsEnd() {
  return status_ == STATUS_SUCCESS || status_ == STATUS_FAILURE;
}

bool Task::IsRunning() {
  return status_ == STATUS_RUNNING;
}

Composit::Composit() {}

Composit::~Composit() {}

void Composit::OnStart() {
  current_pos_ = 0 ? tasks_.size() > 0 : -1;
}

void Composit::OnEnd() {}

void Composit::AddChild(Task* child) {
  tasks_.push_back(std::unique_ptr<Task>(child));
}

void Composit::RemoveChild(int pos) {
  if (pos < 0 || pos >= tasks_.size()) {
    return;
  }
  tasks_.erase(tasks_.begin() + pos);
}

void Composit::ClearChilden() {
  tasks_.clear();
}

Task* Composit::begin() {
  if (tasks_.size() == 0) {
    current_pos_ = -1;
    return nullptr;
  }

  current_pos_ = 0;
  return tasks_[0].get();
}

Task* Composit::current() {
  if (current_pos_ >= 0 && current_pos_ < tasks_.size()) {
    return tasks_[current_pos_].get();
  }
  return nullptr;
}

Task* Composit::next() {
  if (current_pos_ + 1 < tasks_.size()) {
    current_pos_ += 1;
    return current();
  }

  return nullptr;
}

Sequence::Sequence() {}

Sequence::~Sequence() {}

int Sequence::Update() {
  while (true) {
    if (current() == nullptr) {
      return STATUS_FAILURE;
    }

    int res = current()->Tick();
    if (res != STATUS_SUCCESS) {
      return res;
    }

    if (next() == nullptr) {
      return STATUS_SUCCESS;
    }
  }

  return STATUS_FAILURE;
}

Selector::Selector() {}

Selector::~Selector() {}

int Selector::Update() {
  while (true) {
    if (current() == nullptr) {
      return STATUS_FAILURE;
    }

    int res = current()->Tick();
    if (res != STATUS_FAILURE) {
      return res;
    }

    if (next() == nullptr) {
      return STATUS_FAILURE;
    }
  }

  return STATUS_FAILURE;
}

Decorator::Decorator() {}

Decorator::~Decorator() {}

Repeat::Repeat() {}

Repeat::~Repeat() {}

void Repeat::OnStart() {
  count_ = 0;
}

void Repeat::OnEnd() {}

int Repeat::Update() {
  while (count_ < limit_) {
    child()->Tick();
    int child_status = child()->status();
    if (child_status != STATUS_SUCCESS && child_status != STATUS_FAILURE) {
      return child_status;
    }
    ++count_;
  }
  return STATUS_SUCCESS;
}

AlwaysFail::AlwaysFail() {}

AlwaysFail::~AlwaysFail() {}

int AlwaysFail::Update() {
  child()->Tick();
  return STATUS_FAILURE;
}

AlwaysSuccess::AlwaysSuccess() {}

AlwaysSuccess::~AlwaysSuccess() {}

int AlwaysSuccess::Update() {
  child()->Tick();
  return STATUS_SUCCESS;
}

Invert::Invert() {}

Invert::~Invert() {}

int Invert::Update() {
  child()->Tick();
  int child_status = child()->status();
  if (child_status != STATUS_SUCCESS && child_status != STATUS_FAILURE) {
    return child_status;
  }

  if (child()->status() == STATUS_SUCCESS) {
    return STATUS_FAILURE;
  }

  return STATUS_SUCCESS;
}

UntilSuccess::UntilSuccess() {}

UntilSuccess::~UntilSuccess() {}

int UntilSuccess::Update() {
  while (true) {
    child()->Tick();
    int child_status = child()->status();
    if (child_status != STATUS_SUCCESS && child_status != STATUS_FAILURE) {
      return child_status;
    }

    if (child_status == STATUS_SUCCESS) {
      return STATUS_SUCCESS;
    }
  }
}

UntilFail::UntilFail() {}

UntilFail::~UntilFail() {}

int UntilFail::Update() {
  while (true) {
    child()->Tick();
    int child_status = child()->status();
    if (child_status != STATUS_SUCCESS && child_status != STATUS_FAILURE) {
      return child_status;
    }

    if (child_status == STATUS_FAILURE) {
      return STATUS_SUCCESS;
    }
  }
}

Parallel::Parallel()
    : success_policy_(POLICY_NONE),
      failure_policy_(POLICY_NONE) {
}

Parallel::~Parallel() {}

int Parallel::Update() {
  int success_count = 0;
  int failure_count = 0;
  for (Task* task = begin(); task != nullptr; task = next()) {
    if (!task->IsEnd()) {
      task->Tick();
    }

    if (task->status() == STATUS_SUCCESS) {
      ++success_count;
      if (success_policy_ == REQUIRE_ONE) {
        return STATUS_SUCCESS;
      }
    }

    if (task->status() == STATUS_FAILURE) {
      ++failure_count;
      if (failure_policy_ == REQUIRE_ONE) {
        return STATUS_FAILURE;
      }
    }
  }

  if (failure_policy_ == REQUIRE_ALL && failure_count == size()) {
    return STATUS_FAILURE;
  }

  if (success_policy_ == REQUIRE_ALL && success_count == size()) {
    return STATUS_SUCCESS;
  }

  return STATUS_RUNNING;
}

void Parallel::set_policy(Policy success, Policy failure) {
  success_policy_ = success;
  failure_policy_ = failure;
}

void Parallel::OnEnd() {
  Composit::OnEnd();

  for (Task* task = begin(); task != nullptr; task = next()) {
    if (task->IsRunning()) {
      task->Abort();
    }
  }
}
