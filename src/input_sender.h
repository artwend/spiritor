//
// Created by Arthur on 22/12/2020.
//

#ifndef SPIRITOR_INPUT_SENDER_H
#define SPIRITOR_INPUT_SENDER_H

#include "interception.h"

class input_sender {
    InterceptionContext context_;
    const InterceptionDevice& device_;
public:
    input_sender(InterceptionContext context, const InterceptionDevice& device)
    : context_(context), device_(device)
    {}

    void press_key(unsigned short code) const {
        InterceptionKeyStroke stroke{code, INTERCEPTION_KEY_DOWN, 0};
        interception_send(context_, device_, (InterceptionStroke *)&stroke, 1);
    }

    void release_key(unsigned short code) const {
        InterceptionKeyStroke stroke{code, INTERCEPTION_KEY_UP, 0};
        interception_send(context_, device_, (InterceptionStroke *)&stroke, 1);
    }

    void press_adn_release_key(unsigned short code) const {
        InterceptionKeyStroke kstrokes[2] = {
                {code, INTERCEPTION_KEY_DOWN, 0},
                {code, INTERCEPTION_KEY_UP, 0}
        };
        interception_send(context_, device_, (InterceptionStroke *)kstrokes, sizeof kstrokes);
    }
};


#endif //SPIRITOR_INPUT_SENDER_H
