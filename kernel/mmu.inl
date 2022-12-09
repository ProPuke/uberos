namespace mmu {
	extern bool _is_enabled;

	inline auto is_enabled() -> bool {
		return _is_enabled;
	}
}