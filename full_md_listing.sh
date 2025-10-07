#!/usr/bin/env bash

toplevel=$1

printf "# Source dir listing ($toplevel)\n\n"
printf "\`\`\`txt\n%s\n\`\`\`\n" "$(tree --dirsfirst --noreport $toplevel)"

printf "## files content\n\n"

$(bash ./format_sources.sh > /dev/null)

for f in `find $toplevel -name "*.hpp" -o -name "*.cpp" -o -name "*.txx"`;do
    printf "listing: **$f**\n" "$(tree --dirsfirst --noreport $f)"
    printf "\`\`\`cpp\n%s\n\`\`\`\n\n" "$(cat $f)"
done
