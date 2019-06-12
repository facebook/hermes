// RUN: %hermes -lazy %s
// Just make sure this function doesn't crash

function f() {
  return {
    get g() {
      function h() {
        /* Some text to pad out the function so that it won't be eagerly compiled
         * for being too short. Lorem ipsum dolor sit amet, consectetur adipiscing
         * elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
         */
      }
    }
  };
}
print(f().g);
