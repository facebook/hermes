# New Way to Contribute to Hermes

*December 12, 2025 · tmikov*

Contributing to Hermes has been... let's say "challenging." I've been thinking about how to fix that.

## The Problem

The core problem is that Hermes contributions are written against internal APIs. Those APIs are designed for efficiency, not ergonomics, and they're genuinely hard to use correctly. Mistakes can easily lead to serious memory errors - crashes, corruption, security issues. Getting code to production quality against these APIs takes a lot of back-and-forth.

Internal APIs are also unstable - they change all the time as we optimize and refactor. So when we accept a PR, we're not just accepting the code, we're accepting responsibility for maintaining it forever. Every internal API change means we have to update that code too.

And then there's the domain expertise problem. A contribution that implements Intl or the Streams API requires reviewers to understand those specs deeply enough to verify correctness. That expertise often doesn't exist on the team, so either we spend a lot of time building it, or we're not confident in what we're shipping.

The result: PRs sit for months, we ask for endless revisions, and sometimes we just have to say no to perfectly reasonable code because the long-term cost is too high. That sucks for everyone.

## A Different Path: JSI Extensions

Hermes extensions can now be written using JSI - the stable JavaScript Interface API - instead of internal APIs. JSI is designed to be safe and easy to use. And because it doesn't change, code won't break when we refactor Hermes internals.

We've already migrated TextEncoder from internal APIs to JSI as the first real extension - it's a good example of how this works in practice.

## Two Tiers

There are two tiers of JSI extensions. Core extensions live in [`extensions/`](../../API/hermes/extensions/) and are fully maintained by the Hermes team. Community contributions live in [`extensions/contrib/`](../../API/hermes/extensions/contrib/).

For contrib, our commitment is limited: we guarantee that extensions will continue to build and pass the tests they were submitted with, but that's it. Bug reports and support come from the community, not from us.

Because the Hermes team isn't taking on maintenance burden, the review process for contrib is lighter. We focus on structural review - is this good C++, is it using JSI correctly, is the code reasonable? But we're not doing functional review - we're not verifying that an Intl implementation handles every locale correctly, or that a Streams implementation matches the WHATWG spec. We trust contributors to get that right; that's what their tests are for.

## Graduation

If a contrib extension proves itself - gets popular, stays stable, fills a real need - it can graduate to core. That doesn't mean rewriting against internal APIs; it means the extension moves out of contrib and the Hermes team takes over maintenance and support.

## How It Works: TextEncoder Under the Hood

The JavaScript side handles the stuff that's natural in JS - setting up the constructor, defining the prototype, installing on globalThis:

```javascript
extensions.TextEncoder = function(nativeInit, nativeEncode, nativeEncodeInto) {
  function TextEncoder() {
    if (!new.target) {
      throw new TypeError('TextEncoder must be called as a constructor');
    }
    nativeInit(this);
  }

  TextEncoder.prototype.encode = nativeEncode;
  TextEncoder.prototype.encodeInto = nativeEncodeInto;

  // ... property definitions ...

  globalThis.TextEncoder = TextEncoder;
};
```

The C++ side provides the native helper functions that do the actual work - in this case, UTF-8 encoding. These get passed into the JS setup function, which wires them onto the prototype.

This split plays to each language's strengths. JS is great for prototype plumbing and property definitions. C++ handles the performance-critical byte manipulation. And because everything goes through JSI, the C++ side is straightforward and safe - no manual GC interaction, no risk of dangling handles.

The [old internal-API implementation](https://github.com/facebook/hermes/blob/e5d38a7e7f14bcebd00745e662489401f26e63bc/lib/VM/JSLib/TextEncoder.cpp) of TextEncoder was about 340 lines of intricate VM code. The [JSI version](../../API/hermes/extensions/TextEncoder.cpp) is simpler, easier to review, and won't break when we refactor Hermes internals.

## Come Build Something

Docs and a working example are up now. If you've ever wanted to add something to Hermes but got discouraged by the process, this is the path.

- [Extensions README](../../API/hermes/extensions/README.md)
- [Contrib README](../../API/hermes/extensions/contrib/README.md)
