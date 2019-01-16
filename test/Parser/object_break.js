// RUN: (! %hermes %s 2>&1 ) | %FileCheck %s

//CHECK: error: 'break' not within a loop or a switch
{
  while(
    { get 0 () {
      break;
      }
      })
  return 2
}
