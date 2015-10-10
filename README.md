# Pebble Sat

A Pebble Sat to display the next pass of the ISS over your location.

![Pebble Sat showing the time to next pass](screenshot-main.png)

![Pebble Sat during a pass](screenshot-pass.png)

<a href="http://pblweb.com/appstore/52d396683e2e256b6c00001e/">
  <img src="http://pblweb.com/badge/52d396683e2e256b6c00001e/orange/small">
</a>

## How it works

This app fetches the next pass information from a Python app that regularly fetches updated [TLEs](http://en.wikipedia.org/wiki/Two-line_element_set) and calculates the next pass with the [pyephem](http://rhodesmill.org/pyephem/) library. See the [pbsat server](https://github.com/sarfata/pbsat-server) project for more info.

## Attribution

[Map of
World - Single Color](http://www.freevectormaps.com/world-maps/WRLD-EPS-01-0005?ref=atr) by [FreeVectorMaps.com](http://www.freevectormaps.com/?ref=atr).

## License

MIT License

Copyright Thomas Sarlandie 2014

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
