# python-audio

This package is desgined to replace the deprecated audioop package.
Future plan is to make it a one-stop solution for anything audio related for python.

## How to use?

Install using one of the methods below.

```bash
pip install git+https://github.com/AstralScribe/python-audio.git
```

```bash
pip install python-audio
```

## Available features:

- All functionality of audioop has been implemented except three functions `ratecv, adpcm2lin, lin2adpcm`.
  - To use the package just replace `import audioop` with `import audio.op as audioop`
