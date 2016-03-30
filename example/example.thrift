
namespace cpp td.test

enum Numberz
{
  ONE = 1,
  TWO,
  THREE
}

typedef i64 UserId

struct Xtruct
{
  1:  string string_thing
}

struct Insanity
{
  1: map<Numberz, UserId> userMap,
  2: list<Xtruct> xtructs
}

service Example
{
  void NoParamsNoReturn(),
  void HasParamsNoReturn(1: string thing, 2: Xtruct xtruct, 3: Insanity insanity),
  Xtruct NoParamsHasReturn(),
  Insanity HasParamsHasReturn(1: string thing, 2: Xtruct xtruct, 3: Insanity insanity)
}
