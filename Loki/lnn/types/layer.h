#ifndef LAYER_H
#define LAYER_H



template<size_t SIZE, size_t NEXT_SIZE, typename T>
class Layer{
    static_assert(SIZE > 0);
    static_assert(NEXT_SIZE >= 0);

    public:
        Layer();
        ~Layer();

        std::array<T,SIZE> neurons;
        std::array<T,SIZE> biases;

        // For this class, the next Layer's size isn't a constant expression, so we
        // have to declare the weights on heap...
        // They will be declared to be indexed by weights[next_layer_neuron][this_layer_neuron]
        T weights**;

        size_t size() const;
};


#endif