export PYTHONINCLUDE=$(python -c "import sysconfig; print(sysconfig.get_paths()['include'])")
echo $PYTHONINCLUDE
gcc runtests.c -I $PYTHONINCLUDE ../type_inference.c ../blocks.c ../field_types.c ../conversions.c ../str_to.c  ../dtoa_modified.c ../char32utils.c ../max_token_len.c ctestify.c ctestify_assert.c -o runtests
