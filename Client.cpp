# include "Client.hpp"

Client::Client() {
    this->fd = -1;
    this->validPass = false;
}

bool Client::isClientAuth() const {
    return (this->validPass && !this->nickName.empty() && !this->userName.empty());
}
