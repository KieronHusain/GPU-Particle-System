AppendStructuredBuffer<uint>	deadListToAddTo		: register(u0);

[numthreads(256, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	//Adds to the dead list index value
	deadListToAddTo.Append(DTid.x);
}