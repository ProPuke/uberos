I was going with a system whereby all layout controls have `minWidth`, `minHeight`, `maxHeight` and `maxHeight` constraints, and a flexbox `growRatio` for when within containers.
Layout containers obviously have a few additional rules on top, depending on how they layout children, but would place and scale children based on those control constraints.

*However*, I'm realising now that some controls will having differing size constraints depending on their *aspect* ratio - for example linewrapped labels: at a wider width their required vertical space decreases, as more sits on a single line, and at a narrower width their required min y size increases, to accommodate the extra linewrapped lines.
So just the above parameters aren't enough to correctly stack and layout labels that can linewrap (or other more complex controls).
I'm thinking the solution would be 4 additional methods, for querying constraints when the other axis has been locked to a specific size: `minWidthGivenHeight(height)`, `maxWidthGivenHeight(height)`, `minHeightGivenWidth(width)`, `maxHeightGivenWidth(width)`
This would allow controls like linewrapped text to behave correctly within the layout system.
But I'm wondering if there's anything else key I've missed? Is this likely to be enough for now and most layout concerns, or have I missed a more elegant way of thinking about things?

Okay, lots of thinking has occurred.
Web layout has a key difference to UI layout.
In my case I need constraint ranges along both axis. This is so I can feed constraints back and restrict parent window resizing - I must be able to calc the size range of both the X _and_ Y axis (and there is a relationship between the two, which causes problems).
In the case of web layout you can actually only calc constraint ranges on ONE axis - the x. The y axis always reflows on layout to suit the x (this avoids a cross-relationship). And this makes sense as it's a vertically flowing medium. And indeed the same is true of mobile when calculating such constraints.
But that does not suit my needs.

So, in order to easily calc constraints in both dirs I must decouple the relationship between the two.
This means I won't be allowing responsive wordwrap in text labels.
You can have word wrap within a _fixed_ control height OR you can have a scrollable text control with wrapping text within it; just no variable height wordwrap label. And I think you see the same constraint in all native (non-scrolling) UI, so that seems acceptable.

Scrollable content of course has none of these problems - you don't need to know a vertical size _range_ (just a fixed height which is the result of x).

But yeah, keeping shit simple for now.
Any controls that have cross dependencies on their constraints will need to be within scrollable regions.

This gives the following interface per control:
```c
get_min_width()
get_max_width()
get_min_height() // only used for controls not nested in scrollable regions
get_max_height() // only used for controls not nested in scrollable regions
layout(width, optional height) //perform dynamic layout of the control and all children. If a height is included scale height for children using fixed min/max ranges (non-scrolling controls), if a height is not included calc a dynamic respondent height (for scrollable regions). Returns a Measurements struct with a resultant `height` member
```
