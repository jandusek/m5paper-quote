# m5paper-quote

Displays random quotes on an [m5paper](https://shop.m5stack.com/products/m5paper-esp32-development-kit-v1-1-960x540-4-7-eink-display-235-ppi) eInk device.

This assumes existence of a REST API (for example [this one](https://github.com/jandusek/quote-api)) that returns quotes in the following format:

```json
{
  "_id":"63a3063b51d143b33e91af10",
  "quote":"The fact that some geniuses were laughed at does not imply that all who are laughed at are geniuses. They laughed at Columbus, they laughed at Fulton, they laughed at the Wright brothers. But they also laughed at Bozo the Clown.",
  "author":"Carl Sagan"
}
```

## Installation

1. `cp env.h.sample env.h`
2. Edit variables in `env.h`
3. Use Arduino IDE to compile and flash

## Example

![Example](example.jpeg)
