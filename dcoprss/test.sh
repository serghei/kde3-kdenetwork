#!/bin/bash
ID=`dcopstart rssservice`
#dcop $ID RSSService add "http://www.kde.org/dotkdeorg.rdf"
#dcop $ID RSSService add "http://freshmeat.net/backend/fm.rdf"

#dcop $REF1 refresh
#dcop $REF2 refresh
echo "Articles:"
DOCS=`dcop rssservice RSSService list`
for DOC in $DOCS
do
	DOCREF=`dcop rssservice RSSService document "$DOC"`
	TITLE=`dcop $DOCREF title`
	CNT=`dcop $DOCREF count`
	echo $TITLE - $CNT
	echo "------------------------------------"
	while let "CNT >0"
	do
		let "CNT=CNT-1"
		ART=`dcop $DOCREF article $CNT`
		TEXT=`dcop $ART title`
		echo "$CNT	$TEXT"
	done
done
