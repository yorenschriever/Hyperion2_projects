# Pipe based projects

These projects were created before the migration of the core to sender/receiver interfaces, and are still pipe based.

In order to make them work you would need to refactor main.ts to work with the new mechanism of propagating data trough the system. 

## Example

### Old code
```
hyp->addPipe(
    new ConvertPipe<Monochrome,Monochrome12>(
        new PatternInput<Monochrome>(SIZE, new WavePattern()),
        new PWMOutput(1),
        GammaLut12));
```

### New Code
```
hyp->createChain(
    new PatternInput<Monochrome>(SIZE, new WavePattern()),
    new ConvertColor<Monochrome, Monochrome12>(GammaLut12),
    new PWMOutput(1)
);
```

The patterns, maps and other elements can still be re-used one-on-one in new projects.