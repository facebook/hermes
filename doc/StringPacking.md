## How Hermes Packs Strings

This is a description of the string packing algorithm employed by Hermes. Its design is guided by the Hermes optimization principles: it must be predictable and compile-time efficient.

A Hermes bytecode file represents strings as a pair (offset, length) in a big character buffer, which necessarily contains all strings as substrings. By being clever about arranging the strings in the character buffer, we can take advantage of shared substrings. This reduces the size of the bytecode file and the memory footprint of the app.

There are two types of relationships between strings:

1. A full containment relationship. For example, `once upon a time` contains `upon`. In this case, we would like to emit only the characters of the first string, and have the second string reference a substring of the first. We say that the first string is a *parent* of the second.
2. An overlapping relationship: `splitpea` and `peasoup`. Here `pea` is a suffix of the first string and a prefix of the second, and so we would like to emit `splitpeasoup` where the `pea` is shared between strings. We say that the pair (`splitpea`, `peasoup`) has *overlap* of length 3.

The challenge is to find a *superstring* of all our strings which is shorter than simply concatenating them. Finding the shortest such string is the "Shortest Common Superstring" problem, and it is NP-Hard. We employ a greedy heuristic. The output is not as small as the Set Cover approach, but the Set Cover approach appears necessarily quadratic while ours is N log N. (Input size is on the order of 35k strings, so quadratic algorithms are not tolerable.)

The basic approach is inspired by "A greedy approximation algorithm for constructing shortest common superstrings" by Tarhio and Ukkonen.  Given a set of strings S:

1. Consider S to be vertices of a weighted directed graph.
2. Let there be an edge from s1 to s2 if s1 overlaps s2, that is, some string is both a suffix of s1 and a prefix of s2. The weight of the edge is the amount of overlap.
3. Construct a Hamiltonian Path through the graph by greedily choosing edges with the maximum weight, but ignoring edges that would result in a cycle.
4. Output the strings in the order of the path, applying the overlap.

The key bottleneck of the algorithm is step 2: identifying overlapping strings. Tarhio and Ukkonen suggest using KMP string matching on all pairs of strings, which is quadratic. We instead build a generalized suffix array on all strings, which allows both finding parent and overlap relationships via binary search.

### Computing Parents and Overlap

Recall that an overlap is a pair (left, right) where there is some string that is simultaneously a suffix of left, and a prefix of right. For example, `splitpea` and `peasoup` has overlap of 3. We wish to find all parents and all overlaps. We are armed with a suffix array: a sorted list of suffixes of all our strings, where each suffix points back to the string(s) that contain it.

We loop over the right strings, and find all overlapping left mates. For each right string, we loop over its prefixes in increasing length. For example, given `peasoup` we loop over `p`, `pe`, `pea`... Call this the test prefix. We use binary search in our suffix array to identify suffixes that are prefixed by the the test prefix. This set of suffixes is necessarily contiguous, because the suffix array is sorted. If a suffix is exactly equal to that prefix, then we have an overlap. This is necessarily the leftmost element of our range, because our range is sorted.

A key idea is that moving to the next prefix can only narrow the matching suffixes. For example, only suffixes that have `pe` as a prefix may have `pea` as a prefix. Therefore we do not need to reset the binary search range across iterations. Furthermore, for each iteration, we only need to consider one character in each suffix, since we know all previous characters necessarily match. This is key to performance.

When we find nonzero overlap, we add an `Arc` (left -> right) representing edges in our graph. The Arcs are maintained sorted by weight for our greedy algorithm.

#### Example

Say we wish to find overlaps with left string `splitpea` and right string `peasoup` (of course, there is only one overlap of length 3). Our proposed right string is `peasoup` and our suffix array is (considering only the suffixes of `splitpea`):

  `[a, ea, itpea, litpea, pea, plitpea, splitpea, tpea]`

We are going to iterate the prefixes of our right string: `p`, `pe`, `pea`, `peas`, `peaso`, `peasou`, `peasoup`, but we will see that we exit early.

1. Start with `p`. Binary search at index 0 to get the contiguous range `[pea, plitpea]` We then check if the first element is exactly equal to `p` (which we can do simply by checking its length); if it were, we would add an arc of weight 1.
  
2. Next is `pe`. Binary search on index 2 with `e`, yielding a one-element range `[pea]` Again we would add an arc if the first element were equal to `pe`.
  
3. Next is `pea`. Binary search on index 3 with `a`, again yielding `[pea]` The first string of this range is indeed equal to our prefix `pea`, so we add an Arc `peasoup -> splitpea` with overlap 3.
  
4. Next is `peas`. Binary search to narrow our range by the fourth character `s`. This yields an empty range, so we exit the loop.

After exhausting our prefixes, it may happen that our range of suffixes is not empty. For example, if we searched for `lit` we would end up with one suffix `litpea`. This is how we identify substring containment relationships. In this case we don't add an arc (since there's not actually any overlap) but we do set a pointer from `lit` to the parent string that contains it, and the offset within that parent.

#### Suffix Array Construction Optimizations

We construct the suffix array by uniquing suffixes via a hash set, and then sorting them. This was found to be faster than using a sorted data structure (such as `std::map`) because comparing long strings lexicographically is expensive.

There are some optimizations we take advantage of:

1. We are only interested in suffixes that share a prefix with some string in our set. For example, for the set [splitpea, peasoup] we only need to look at suffixes that start with s or p. We take this a bit further by making a hash set of three-character prefixes of our strings ("trigrams"), and only constructing suffixes prefixed by one of those trigrams. This causes us to miss overlaps of length 1 and 2, but massively reduces the number of suffixes that we have to consider.
2. There are many long strings with shared suffixes. When constructing the suffix array, using a three-way radix quicksort instead of `std::sort()` is a substantial win, because it avoids comparing known-matching prefixes.

#### Further Optimization

1. Computing overlap is a natural candidate to be done in parallel.
2. It's possible that we could employ a faster algorithm for suffix array construction such as SA-IS, but it is unclear how we would unique our suffixes with that approach.

### References

1. ["A greedy approximation algorithm for constructing shortest common superstrings" by Tarhio and Ukkonen](http://www.sciencedirect.com/science/article/pii/0304397588901673)
2. [3-way radix quicksort](https://en.wikipedia.org/wiki/Multi-key_quicksort)
