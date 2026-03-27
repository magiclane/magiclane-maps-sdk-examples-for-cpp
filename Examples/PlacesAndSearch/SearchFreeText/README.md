# Search free text

Search for places using text.

![](screenshot.png)

## Use case

Search for points of interest relevant to the given input text.

## How to use the sample

When you run the example app, you'll be viewing the scene from above. A fly will be performed to the first point of interest found.

## How it works

1. Create a `MapServiceListener`, `OpenGLContext`, `Screen` and `MapView`.
2. Create a `LandmarkList`.
3. Call the `SearchService` using the list from 2. a progress listener, the keywords you are searching for and a pair of coordinates relevant to your search.
4. Once the search operation completes, instruct the `MapView` to center on the coordinates of the first result.
5. Instruct the `MapView` to activate the highlight in order for the result to be seen better.
