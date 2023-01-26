# m5paper-quote

Displays random quotes on an [m5paper](https://shop.m5stack.com/products/m5paper-esp32-development-kit-v1-1-960x540-4-7-eink-display-235-ppi) eInk device.

This assumes existence of a REST API that returns quotes in the following format:

```json
{
  "_id":"63d0bab591bcfcf03daea983",
  "quote":"No man chooses evil because it is evil; he only mistakes it for happiness, the good he seeks.",
  "author":"Mary Wollstonecraft"
}
```

## Installation

1. `cp env.h.sample env.h`
2. Edit variables in `env.h`
3. Use Arduino IDE to compile and flash
