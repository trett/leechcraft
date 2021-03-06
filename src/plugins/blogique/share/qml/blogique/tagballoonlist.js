var balloonArray = new Array ();
var currentBalloon;

function addBalloon (balloon)
{
	balloonArray.push (balloon)
}

function removeBalloon (balloon)
{
	var ballon = balloonArray.indexOf (balloon);
	balloonArray.splice (balloonArray.indexOf (balloon), 1);
	balloon.opacity = 0.0;
	balloon.destroy ();
}

function getAllBalloones ()
{
	return balloonArray;
}

function getLastBalloon ()
{
	return balloonArray [balloonArray.length - 1];
}

function setBalloonAsCurrent (balloon)
{
	currentBalloon = balloon;
}

function getCurrentBalloon ()
{
	return currentBalloon;
}

function count ()
{
	return balloonArray.length;
}

function contains (tagName)
{
	var length = balloonArray.length;
	for (var i = 0; i < length; ++i)
		if (balloonArray [i].tag == tagName)
			return true;

	return false;
}

function getBalloonByName (tagName)
{
	var length = balloonArray.length;
	for (var i = 0; i < length; ++i)
		if (balloonArray [i].tag == tagName)
			return balloonArray [i];
	return null;
}