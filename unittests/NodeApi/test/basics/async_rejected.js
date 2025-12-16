function resolveAfterTimeout() {
  return new Promise((_, reject) => {
    setTimeout(() => {
      reject("test async rejected");
    }, 0);
  });
}

async function asyncCall() {
  console.log("test async calling");
  try {
    const result = await resolveAfterTimeout();
    console.log(`Unexpected: ${result}`);
  } catch (error) {
    console.error(`Expected: ${error}`);
  }
}

asyncCall();