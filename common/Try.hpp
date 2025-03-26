#pragma once

template <typename Result = void, typename Error = void>
struct [[nodiscard]] Try {
	Result result;
	Error error;
	const char *errorMessage = nullptr;

	/**/ Try(Result result):result(result){}
	/**/ Try(Error error, const char *errorMessage):error(error),errorMessage(errorMessage){}
	/**/ Try(Try<void, Error> failure):error(failure,error), errorMessage(failure.errorMessage){}

	explicit operator bool() { return errorMessage==nullptr; }

	auto as_error_only() -> Try<void, Error> { return {error, errorMessage}; }

	template <typename Cast>
	auto cast() -> Try<Cast, Error> { return errorMessage?Try<Cast, Error>{error, errorMessage}:Try<Cast, Error>{(Cast)result}; }
};

template <>
struct [[nodiscard]] Try<void, void> {
	const char *errorMessage = nullptr;

	explicit operator bool() { return errorMessage==nullptr; }
};

template <typename Result>
struct [[nodiscard]] Try<Result, void> {
	Result result;
	const char *errorMessage = nullptr;

	/**/ Try(Result result):result(result){}
	/**/ Try(Try<void, void> failure):errorMessage(failure.errorMessage){}
	/**/ Try(const char *errorMessage):errorMessage(errorMessage){}

	explicit operator bool() { return errorMessage==nullptr; }

	auto as_error_only() -> Try<void, void> { return {errorMessage}; }

	template <typename Cast>
	auto cast() -> Try<Cast, void> { return errorMessage?Try<Cast, void>{errorMessage}:Try<Cast, void>{(Cast)result}; }
};

template <typename Error>
struct [[nodiscard]] Try<void, Error> {
	Error error;
	const char *errorMessage = nullptr;

	explicit operator bool() { return errorMessage==nullptr; }
};

#define TRY(ACTION) do{\
	if(auto action = (ACTION); !action) return action;\
}while(false)
#define TRY_IGNORE(ACTION) ((void)(ACTION))
#define TRY_RESULT(ACTION) ({ auto action = (ACTION); if(!action) return action.as_error_only(); action.result; })
#define TRY_RESULT_CAST(TYPE, ACTION) ({ auto action = (ACTION); if(!action) return action.as_error_only_cast<TYPE>(); (TYPE)action.result; })
#define TRY_RESULT_OR(ACTION, OR) ({ auto action = (ACTION); action?action.result:(OR); })
#define TRY_RESULT_OR_RETURN(ACTION, OR) ({ auto action = (ACTION); if(!action) return (OR); action.result; })
