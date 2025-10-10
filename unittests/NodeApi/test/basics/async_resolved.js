function resolveAfterTimeout() {
  return new Promise((resolve) => {
    setTimeout(() => {
      resolve("test async resolved");
    }, 0);
  });
}

async function asyncCall() {
  console.log("test async calling");
  const result = await resolveAfterTimeout();
  console.log(`Expected: ${result}`);
}

asyncCall();