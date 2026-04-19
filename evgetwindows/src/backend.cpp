#include "evgetwindows/backend.h"

#include <memory>
#include <string>

#include "evget/error.h"
#include "evget/storage/store.h"

evget::Result<std::unique_ptr<evgetwindows::Backend>> evgetwindows::Backend::Create(
    evget::Store& /*storage*/
) {
    return evget::Err{
        evget::Error<evget::ErrorType>{
            .error_type = evget::ErrorType::kEventHandlerError,
            .message = std::string{"Windows backend under development"},
        },
    };
}
